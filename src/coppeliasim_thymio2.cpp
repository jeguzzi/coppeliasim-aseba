#include "coppeliasim_thymio2.h"
#include "logging.h"

#include <math.h>

#define G 9.81f

#define TEXTURE_SIZE 1024

static cv::Mat body_texture;
static std::array<cv::Mat, 3> led_texture_images;
static bool loaded_textures = false;

namespace CS {

// Thymio behaviors runs at 50Hz in the firmware.

class Thymio2::Behavior {
 private:
  Thymio2 & robot;
  unsigned int behavior;
  int acc_previous_led;
  float bat_counter;
  float button_intensity[Button::COUNT];
  float temp_counter;
  float sound_led_intensity;
  bool enabled(int b) { return behavior & b; }
  static constexpr float bat_period = 0.12f;  // s
  static constexpr float button_delta = 4.6875f;  // intensity/s
  static constexpr float temp_hot = 28.0f;  // C
  static constexpr float temp_cold = 15.0f;  // C
  static constexpr float temp_period = 0.5f;  // s
  static constexpr float bat_high = 3.9f;
  static constexpr float bat_middle = 3.6f;
  static constexpr float bat_low = 3.4f;

  void battery(float time_step) {
    float bat = robot.get_battery_voltage();
    float intensity[3] = {1.0f, (bat > bat_middle) ? 1.0f : 0.0f, (bat > bat_high) ? 1.0f : 0.0f};
    if (bat <= bat_low) {
      // 60 ms on + 60 ms off
      intensity[0] = (bat_counter <= bat_period / 2);
      bat_counter += time_step;
      if (bat_counter > bat_period) {
        bat_counter = 0;
      }
    }
    robot.set_led_intensity(LED::BATTERY_0, intensity[0]);
    robot.set_led_intensity(LED::BATTERY_1, intensity[1]);
    robot.set_led_intensity(LED::BATTERY_2, intensity[2]);
  }

  void sound_mic(float time_step) {
    // In the real firmware, it runs at 60 Hz in the loop that computes the sound mean value,
    // and possibly emit the event.
    float t = robot.get_mic_threshold();
    float p = sound_led_intensity;
    if (t) {
      float i = robot.get_mic_intensity();
      if (i > t && i > sound_led_intensity) {
        sound_led_intensity = i;
      } else if (sound_led_intensity) {
        sound_led_intensity = std::max(0.0, sound_led_intensity - time_step * 60.0 / 255.0);
      }
    } else {
      sound_led_intensity = 0.0;
    }
    if (p != sound_led_intensity) {
      robot.set_led_intensity(LED::RIGHT_BLUE, sound_led_intensity);
    }
  }

  void leds_buttons(float time_step) {
    for (int i = 0; i < Button::COUNT; i++) {
      if (robot.get_button(i)) {
        // + 0.09375 after 20 ms
        button_intensity[i] +=  button_delta * time_step;
        if (button_intensity[i] > 1.0)
          button_intensity[i] = 1.0;
      } else {
        button_intensity[i] = 0.0;
      }
    }
    if (button_intensity[2]) {
      for (int i = 0; i < 4; i++)
        robot.set_led_intensity(LED::BUTTON_UP + i, button_intensity[Button::CENTER]);
    } else {
      robot.set_led_intensity(LED::BUTTON_DOWN, button_intensity[Button::BACKWARD]);
      robot.set_led_intensity(LED::BUTTON_LEFT, button_intensity[Button::LEFT]);
      robot.set_led_intensity(LED::BUTTON_UP, button_intensity[Button::FORWARD]);
      robot.set_led_intensity(LED::BUTTON_RIGHT, button_intensity[Button::RIGHT]);
    }
  }

  void leds_prox(float time_step) {
    float h = CS::ProximitySensor::max_value - CS::ProximitySensor::min_value;
    // Do a linear transformation from min-max to led 0-31!
    for (int i = 0; i < 7; i++) {
      float intensity = (robot.get_proximity_value(i) - CS::ProximitySensor::min_value) / h;
      if (i < 2) {
        robot.set_led_intensity(CS::LED::IR_FRONT_0 + i, intensity);
      }
      if (i >= 2) {
        robot.set_led_intensity(CS::LED::IR_FRONT_0 + i + 1, intensity);
      }
    }
    for (int i = 0; i < 2; i++) {
      float intensity = robot.get_ground_delta(i) / CS::GroundSensor::max_value;
      robot.set_led_intensity(CS::LED::IR_GROUND_0 + i, intensity);
    }
  }

  void leds_acc(float time_step) {
    // RING_0 is at the front, then clockwise.
    float intensity = 0.0;
    int index = -1;
    const float * acc = robot.get_acceleration_values();
    if (acc_previous_led >= 0) {
      robot.set_led_intensity(LED::RING_0 + acc_previous_led, 0);
      acc_previous_led = -1;
    }
    if (acc[2] < (G * 21.0f / 23.0f)) {
      index = round(atan2(acc[1], acc[0]) / M_PI * 4);
      if ((abs(acc[0])+ abs(acc[1])) <= 10.0 * G / 23.0) {
        intensity = 0.0;
      } else {
        intensity = std::clamp(1.3125 - abs(acc[2] / G * 1.4375), 0.0, 1.0);
      }
      index = 4 - index;
      if (index > 7) {
        index -= 8;
      } else if (index < 0) {
        index += 8;
      }
      printf("A %d \n", index);
      robot.set_led_intensity(LED::RING_0 + index, intensity);
      acc_previous_led = index;
    }
  }

  void leds_rc5(float time_step) {
    robot.set_led_intensity(LED::RIGHT_RED, robot.received_rc_message() ? 1.0 : 0.0);
  }

  void leds_ntc(float time_step) {
    temp_counter += time_step;
    if (temp_counter > temp_period) {
      float temperature = robot.get_temperature();
      temp_counter = 0;
      float r = std::clamp((temperature - temp_cold) / (temp_hot - temp_cold), 0.0f, 1.0f);
      robot.set_led_color(LED::LEFT_BLUE, false, r, 0.0, 1.0 - r);
    }
  }

 public:
  explicit Behavior(Thymio2 & robot, uint8_t mask = 0x0) :
    robot(robot), behavior(mask), acc_previous_led(-1),
    sound_led_intensity(0) {}

  void set(uint8_t value) {
    behavior = value;
  }

  void set_enable(bool value, uint8_t mask) {
    if (value)
      behavior |= mask;
    else
      behavior &= ~mask;
  }
  void do_step(float dt)  {
    if (!behavior) return;

    if (enabled(BEHAVIOR::LEDS_BATTERY))
      battery(dt);

    if (enabled(BEHAVIOR::LEDS_BUTTON))
      leds_buttons(dt);

    if (enabled(BEHAVIOR::LEDS_MIC))
      sound_mic(dt);

    if (enabled(BEHAVIOR::LEDS_PROX))
      leds_prox(dt);

    if (enabled(BEHAVIOR::LEDS_RC5))
      leds_rc5(dt);

    if (enabled(BEHAVIOR::LEDS_ACC))
      leds_acc(dt);

    if (enabled(BEHAVIOR::LEDS_NTC))
      leds_ntc(dt);

    // if(ENABLED(B_LEDS_SD))
    //   behavior_sd();
    // if(ENABLED(B_SOUND_Button))
    //   behavior_sound_buttons();
    // if (ENABLED(B_MODE))
    //   mode_tick();
  }
};


enum {
  TOP_TEXTURE,
  BOTTOM_TEXTURE,
  LED_TEXTURE
};

class TextureRegion {
 public:
  unsigned x, y, w, h;
  TextureRegion(unsigned _x, unsigned _y, unsigned _w, unsigned _h) :
    x(_x), y(_y), w(_w), h(_h) {}
};

class LEDTexture {
 public:
  std::vector<TextureRegion> regions;
  std::vector<size_t> overlapping_leds;
  unsigned texture_index;
  LEDTexture(std::vector<TextureRegion> regions, unsigned index, std::vector<size_t> leds = {}) :
    regions(regions), texture_index(index), overlapping_leds(leds) {}
};

std::array<LEDTexture, LED::COUNT> led_textures = {
  LEDTexture{{{130, 535, 350, 488}, {485, 630, 500, 100},
              {480, 846, 265, 50}, {790, 160, 50, 265}
            }, TOP_TEXTURE,
            {LED::BUTTON_DOWN, LED::BUTTON_LEFT, LED::BUTTON_RIGHT,
             LED::RING_2, LED::RING_3, LED::RING_4, LED::RING_5, LED::RING_6,
             LED::IR_BACK_0, LED::IR_BACK_1,
             LED::LEFT_RED, LED::LEFT_BLUE, LED::RIGHT_BLUE, LED::RIGHT_RED,
             LED::BATTERY_0, LED::BATTERY_1, LED::BATTERY_2}},
  LEDTexture{{{563, 38, 117, 301}, {651, 731, 210, 113}}, BOTTOM_TEXTURE,
             {LED::IR_FRONT_0, LED::IR_FRONT_1, LED::IR_FRONT_2, LED::IR_FRONT_3,
              LED::LEFT_RED, LED::LEFT_BLUE, LED::RIGHT_BLUE, LED::RIGHT_RED}},
  LEDTexture{{{565, 344, 224, 192}}, BOTTOM_TEXTURE,
             {LED::IR_FRONT_2, LED::IR_FRONT_3, LED::IR_FRONT_4, LED::IR_FRONT_5,
              LED::LEFT_RED, LED::LEFT_BLUE, LED::RIGHT_BLUE, LED::RIGHT_RED}},
  LEDTexture{{{82, 759, 36, 47}}, LED_TEXTURE},
  LEDTexture{{{160, 759, 36, 47}}, LED_TEXTURE},
  LEDTexture{{{116, 803, 47, 36}}, LED_TEXTURE},
  LEDTexture{{{116, 725, 47, 36}}, LED_TEXTURE},
  LEDTexture{{{11, 741, 41, 82}}, LED_TEXTURE},
  LEDTexture{{{33, 677, 67, 67}}, LED_TEXTURE},
  LEDTexture{{{98, 654, 82, 41}}, LED_TEXTURE},
  LEDTexture{{{177, 677, 67, 67}}, LED_TEXTURE},
  LEDTexture{{{226, 741, 41, 82}}, LED_TEXTURE},
  LEDTexture{{{177, 821, 67, 67}}, LED_TEXTURE},
  LEDTexture{{{98, 869, 82, 41}}, LED_TEXTURE},
  LEDTexture{{{33, 821, 67, 67}}, LED_TEXTURE},
  LEDTexture{{{541, 16, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{547, 100, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{550, 219, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{552, 282, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{549, 405, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{544, 500, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{866, 613, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{527, 613, 62, 62}}, LED_TEXTURE},
  LEDTexture{{{694, 818, 79, 90}}, LED_TEXTURE},
  LEDTexture{{{694, 818, 79, 90}}, LED_TEXTURE},
  LEDTexture{{{769, 337, 70, 89}}, LED_TEXTURE},
  LEDTexture{{{775, 426, 50, 50}}, LED_TEXTURE},
  LEDTexture{{{659, 167, 32, 36}}, LED_TEXTURE},
  LEDTexture{{{659, 374, 32, 36}}, LED_TEXTURE},
  LEDTexture{{{271, 762, 28, 13}}, LED_TEXTURE},
  LEDTexture{{{271, 775, 28, 12}}, LED_TEXTURE},
  LEDTexture{{{271, 787, 28, 12}}, LED_TEXTURE}
};


// generated with this Python code:
// str(', ').join(map(lambda x: str(int(x*255)), pow(np.arange(0., 1.001, 1./255.), 0.30)))
static const uint32_t pow_030_table[256] = {
  0, 48, 59, 67, 73, 78, 82, 86, 90, 93, 96, 99, 101, 104, 106, 108, 111, 113, 115, 117, 118, 120,
  122, 123, 125, 127, 128, 130, 131, 132, 134, 135, 136, 138, 139, 140, 141, 142, 144, 145, 146,
  147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 160, 161, 162, 163, 164,
  165, 166, 166, 167, 168, 169, 169, 170, 171, 172, 173, 173, 174, 175, 175, 176, 177, 178, 178,
  179, 180, 180, 181, 182, 182, 183, 184, 184, 185, 185, 186, 187, 187, 188, 189, 189, 190, 190,
  191, 191, 192, 193, 193, 194, 194, 195, 195, 196, 197, 197, 198, 198, 199, 199, 200, 200, 201,
  201, 202, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 209, 210, 210,
  211, 211, 212, 212, 213, 213, 213, 214, 214, 215, 215, 216, 216, 217, 217, 217, 218, 218, 219,
  219, 220, 220, 220, 221, 221, 222, 222, 222, 223, 223, 224, 224, 224, 225, 225, 226, 226, 226,
  227, 227, 228, 228, 228, 229, 229, 230, 230, 230, 231, 231, 231, 232, 232, 233, 233, 233, 234,
  234, 234, 235, 235, 236, 236, 236, 237, 237, 237, 238, 238, 238, 239, 239, 239, 240, 240, 240,
  241, 241, 241, 242, 242, 242, 243, 243, 243, 244, 244, 244, 245, 245, 245, 246, 246, 246, 247,
  247, 247, 248, 248, 248, 249, 249, 249, 250, 250, 250, 251, 251, 251, 251, 252, 252, 252, 253,
  253, 253, 254, 254, 254, 255 };
static const uint32_t pow_035_table[256] = { 0, 36, 46, 53, 59, 64, 68, 72, 75, 79, 82, 84, 87, 89,
  92, 94, 96, 98, 100, 102, 104, 106, 108, 109, 111, 113, 114, 116, 117, 119, 120, 121, 123, 124,
  125, 127, 128, 129, 130, 132, 133, 134, 135, 136, 137, 138, 140, 141, 142, 143, 144, 145, 146,
  147, 148, 149, 150, 150, 151, 152, 153, 154, 155, 156, 157, 158, 158, 159, 160, 161, 162, 163,
  163, 164, 165, 166, 166, 167, 168, 169, 169, 170, 171, 172, 172, 173, 174, 175, 175, 176, 177,
  177, 178, 179, 179, 180, 181, 181, 182, 183, 183, 184, 185, 185, 186, 186, 187, 188, 188, 189,
  189, 190, 191, 191, 192, 192, 193, 194, 194, 195, 195, 196, 197, 197, 198, 198, 199, 199, 200,
  200, 201, 201, 202, 203, 203, 204, 204, 205, 205, 206, 206, 207, 207, 208, 208, 209, 209, 210,
  210, 211, 211, 212, 212, 213, 213, 214, 214, 215, 215, 216, 216, 217, 217, 218, 218, 218, 219,
  219, 220, 220, 221, 221, 222, 222, 223, 223, 223, 224, 224, 225, 225, 226, 226, 227, 227, 227,
  228, 228, 229, 229, 230, 230, 230, 231, 231, 232, 232, 232, 233, 233, 234, 234, 235, 235, 235,
  236, 236, 237, 237, 237, 238, 238, 239, 239, 239, 240, 240, 240, 241, 241, 242, 242, 242, 243,
  243, 244, 244, 244, 245, 245, 245, 246, 246, 247, 247, 247, 248, 248, 248, 249, 249, 250, 250,
  250, 251, 251, 251, 252, 252, 252, 253, 253, 253, 254, 254, 255 };
static const uint32_t pow_040_table[256] = { 0, 27, 36, 43, 48, 52, 56, 60, 63, 66, 69, 72, 75, 77,
  79, 82, 84, 86, 88, 90, 92, 93, 95, 97, 99, 100, 102, 103, 105, 106, 108, 109, 111, 112, 113, 115,
  116, 117, 119, 120, 121, 122, 123, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 136, 137,
  138, 139, 140, 141, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 152, 153, 154,
  155, 156, 157, 157, 158, 159, 160, 161, 161, 162, 163, 164, 165, 165, 166, 167, 168, 168, 169,
  170, 171, 171, 172, 173, 173, 174, 175, 176, 176, 177, 178, 178, 179, 180, 180, 181, 182, 182,
  183, 184, 184, 185, 186, 186, 187, 187, 188, 189, 189, 190, 191, 191, 192, 192, 193, 194, 194,
  195, 195, 196, 197, 197, 198, 198, 199, 200, 200, 201, 201, 202, 202, 203, 204, 204, 205, 205,
  206, 206, 207, 207, 208, 208, 209, 210, 210, 211, 211, 212, 212, 213, 213, 214, 214, 215, 215,
  216, 216, 217, 217, 218, 218, 219, 219, 220, 220, 221, 221, 222, 222, 223, 223, 224, 224, 225,
  225, 226, 226, 227, 227, 228, 228, 229, 229, 229, 230, 230, 231, 231, 232, 232, 233, 233, 234,
  234, 235, 235, 235, 236, 236, 237, 237, 238, 238, 239, 239, 239, 240, 240, 241, 241, 242, 242,
  242, 243, 243, 244, 244, 245, 245, 245, 246, 246, 247, 247, 248, 248, 248, 249, 249, 250, 250,
  250, 251, 251, 252, 252, 252, 253, 253, 254, 254, 255 };

static void draw_rect(uint8_t* target, uint8_t* base, uint32_t * diff, int width,
                      int x0, int y0, int w, int h,
                      float colorR, float colorG, float colorB, float colorA,
                      uint8_t* cache) {
  uint8_t *t = target;
  uint8_t * c = nullptr;
  for (int y = h-1; y >= 0; y--) {
    // uint8_t *t = target + y * width * 3;
    for (int x = 0; x < w; x++) {
      const size_t index(x + x0 + width * (y + y0));
      // expand destination (prev color) into its components
      const uint32_t baseR(base[3 * index+2]);
      const uint32_t baseG(base[3 * index+1]);
      const uint32_t baseB(base[3 * index]);
      // expand diffuse into its components
      const uint32_t diffuse(diff[index]);
      const uint32_t diffA((diffuse>>24) & 0xff);
      const uint32_t diffR((diffuse>>16) & 0xff);
      const uint32_t diffG((diffuse>>8)  & 0xff);
      const uint32_t diffB((diffuse>>0)  & 0xff);
      // compute source color
      // gamma correction because LEDs have non-linear transfer functions
      // const uint32_t sourceA((colorA * diffA) >> 8);
      const uint32_t sourceA = (uint8_t)(diffA * colorA);
      const uint32_t sourceR(pow_035_table[(uint8_t)(colorR * diffR)]);
      const uint32_t sourceG(pow_030_table[(uint8_t)(colorG * diffG)]);
      const uint32_t sourceB(pow_040_table[(uint8_t)(colorB * diffB)]);
      const uint32_t oneMSrcA(255-sourceA);
      // blend color
      //
      const uint8_t r = *t++ = ((baseR * oneMSrcA + sourceR * sourceA) >> 8);
      const uint8_t g = *t++ =((baseG * oneMSrcA + sourceG * sourceA) >> 8);
      const uint8_t b = *t++ = ((baseB * oneMSrcA + sourceB * sourceA) >> 8);
      if (cache) {
        cache[3 * index] = b;
        cache[3 * index+1] = g;
        cache[3 * index+2] = r;
      }
    }
  }
}

static void load_textures() {
  if (loaded_textures) return;
  body_texture = cv::imread(
      TEXTURE_DIR "/thymio-body-texture.png");
  led_texture_images[TOP_TEXTURE] = cv::imread(
      TEXTURE_DIR "/thymio-body-diffusionMap0.png", cv::IMREAD_UNCHANGED);
  led_texture_images[BOTTOM_TEXTURE] = cv::imread(
      TEXTURE_DIR "/thymio-body-diffusionMap1.png", cv::IMREAD_UNCHANGED);
  led_texture_images[LED_TEXTURE] = cv::imread(
      TEXTURE_DIR "/thymio-body-diffusionMap2.png", cv::IMREAD_UNCHANGED);
  loaded_textures = true;
}

static std::array<std::string, 2> wheel_prefixes = {"/Left", "/Right"};
static std::array<std::string, 7> proximity_names = {
    "Left", "CenterLeft", "Center", "CenterRight", "Right", "RearLeft", "RearRight"};
static std::array<std::string, 2> ground_names = {"Left", "Right"};
static std::array<std::string, Button::COUNT> button_names = {
    "Backward", "Left", "Center", "Forward", "Right"};

Thymio2::Thymio2(simInt handle_, uint8_t default_behavior_mask_) :
  Robot(handle_), default_behavior_mask(default_behavior_mask_),
  behavior(new Behavior(*this, default_behavior_mask_)),
  battery_voltage(3.61), temperature(22.0), mic_intesity(0.0), mic_threshold(0.0), r5(false),
  sd_card() {
  simChar * alias = simGetObjectAlias(handle, 2);
  std::string body_path = std::string(alias)+"/Body";
  body_handle = simGetObject(body_path.c_str(), -1, -1, 0);
  texture_id = simGetShapeTextureId(body_handle);
  log_info("Initializing Thymio2 with handle %d and texture_id %d", handle, texture_id);
  for (const auto & wheel_prefix : wheel_prefixes) {
    std::string wheel_path = std::string(alias) + wheel_prefix + "Motor";
    simInt wheel_handle = simGetObject(wheel_path.c_str(), -1, -1, 0);
    wheels.push_back(Wheel(0.027, wheel_handle));
  }
  for (size_t i = 0; i < proximity_names.size(); i++) {
    std::string prox_path = std::string(alias)+"/Proximity" + proximity_names[i];
    simInt prox_handle = simGetObject(prox_path.c_str(), -1, -1, 0);
    proximity_sensors.push_back(ProximitySensor(prox_handle));
    prox_comm.emitter_handles[i] = prox_handle;
    prox_comm.sensor_handles[i] = simGetObject((prox_path + "/Comm").c_str(), -1, -1, 0);
  }
  for (const auto & ground_name : ground_names) {
    std::string ground_path = std::string(alias)+"/Ground" + ground_name;
    simInt ground_handle = simGetObject(ground_path.c_str(), -1, -1, 0);
    ground_sensors.push_back(GroundSensor(ground_handle));
  }
  std::string acc_path = std::string(alias)+"/Accelerometer";
  simInt acc_handle = simGetObject(acc_path.c_str(), -1, -1, 0);
  accelerometer = Accelerometer(acc_handle);
  for (size_t i = LED::BUTTON_UP; i <= LED::BUTTON_RIGHT; i++) {
    leds[i].color = Color(1, 0, 0);
  }
  for (size_t i = LED::RING_0; i <= LED::RING_7; i++) {
    leds[i].color = Color(1, 0.5, 0);
  }
  for (size_t i = LED::IR_FRONT_0; i <= LED::IR_BACK_1; i++) {
    leds[i].color = Color(1, 0, 0);
  }
  leds[LED::RIGHT_RED].color = Color(1, 0, 0);
  leds[LED::LEFT_BLUE].color = Color(0, 1, 1);
  leds[LED::RIGHT_BLUE].color = Color(0, 1, 1);
  for (size_t i = LED::IR_GROUND_0; i <= LED::IR_GROUND_1; i++) {
    leds[i].color = Color(1, 0, 0);
  }
  for (size_t i = LED::BATTERY_0; i <= LED::BATTERY_2; i++) {
    leds[i].color = Color(0.75, 1, 0);
  }

  for (size_t i = 0; i < buttons.size(); i++) {
    std::string button_path = std::string(alias)+"/Button" + button_names[i];
    simInt button_handle = simGetObject(button_path.c_str(), -1, -1, 0);
    buttons[i] = Button(button_handle);
  }

  simReleaseBuffer(alias);
  reset();
}

Thymio2::~Thymio2() {
  reset_texture(false);
}

void Thymio2::enable_behavior(bool value, uint8_t mask) {
  behavior->set_enable(value, mask);
}

void Thymio2::reset_texture(bool reload) {
  load_textures();
  texture = cv::Mat(TEXTURE_SIZE, TEXTURE_SIZE, CV_8UC3);
  cv::cvtColor(body_texture, texture, cv::COLOR_BGR2RGB);
  cv::Mat m = cv::Mat(TEXTURE_SIZE, TEXTURE_SIZE, CV_8UC3);
  cv::flip(texture, m, 0);
  simInt64 uid = simGetObjectUid(handle);
  // HACK(Jerome): One pixel should be specific to each robot,
  // else coppeliaSim will link them when it save the scene
  // But this hack isnot working for image loaded textures
  // log_debug("hack: add pixel with uid %lld\n", uid);
  m.data[0] = (uint8_t) (uid & 0xFF);
  m.data[1] = (uint8_t) ((uid >> 8) & 0xFF);
  m.data[2] = (uint8_t) ((uid >> 16) & 0xFF);
  if (reload) {
    // simInt textureResolution[2] = {TEXTURE_SIZE, TEXTURE_SIZE};
    SShapeVizInfo info;
    simInt r = simGetShapeViz(body_handle, 0, &info);
    // printf("r %d, (%d, %d) %d \n", r, info.textureRes[0], info.textureRes[1], info.indicesSize);
    texture_id = simApplyTexture(
        body_handle, info.textureCoords, info.indicesSize * 2,
        (const simUChar *)m.ptr(), info.textureRes, 1);
    // bottom-left corner is not mapped on the shape (i.e., the pixel is not visible)

    log_debug("Reset texture for handle %d -> texture_id %d", handle, texture_id);
    simReleaseBuffer((const char *)info.indices);
    simReleaseBuffer((const char *)info.normals);
    simReleaseBuffer((const char *)info.texture);
    simReleaseBuffer((const char *)info.textureCoords);
  } else {
    simWriteTexture(texture_id, 0, (const char *)m.ptr(), 0, 0, TEXTURE_SIZE, TEXTURE_SIZE, 0);
  }
}

void Thymio2::reset() {
  Robot::reset();
  for (auto & led : leds) {
    led.color.a = 0;
  }
  reset_texture(true);
  set_prox_comm_tx(0);
  enable_prox_comm(false);
  set_mic_threshold(0);
  set_mic_intensity(0);
  r5 = false;
  temperature = 22.0;
  battery_voltage = 3.61;
  behavior->set(default_behavior_mask);
  sd_card.close();
  log_info("Reset Thymio");
}

void Thymio2::set_led_intensity(size_t index, float intensity) {
  if (index >= LED::COUNT) return;
  LED & led = leds[index];
  if (led.color.set_a(std::clamp(intensity, 0.0f, 1.0f))) {
    set_led_color(index, true);
  }
}

float Thymio2::get_led_intensity(size_t index) const {
  if (index >= LED::COUNT) return 0.0;
  const LED & led = leds[index];
  return led.color.a;
}

float Thymio2::get_led_channel(size_t index, size_t channel) const {
  if (index >= LED::COUNT) return 0.0;
  const LED & led = leds[index];
  switch (channel) {
    case 0:
      return led.color.r;
    case 1:
      return led.color.g;
    case 2:
      return led.color.b;
    default:
      return 0;
  }
}

void Thymio2::set_led_channel(size_t index, size_t channel, float value) {
  if (index >= LED::COUNT) return;
  LED & led = leds[index];
  float r = led.color.r * led.color.a;
  float g = led.color.g * led.color.a;
  float b = led.color.b * led.color.a;
  switch (channel) {
    case 0:
      r = std::clamp<float>(value, 0, 1);
      break;
    case 1:
      g = std::clamp<float>(value, 0, 1);
      break;
    case 2:
      b = std::clamp<float>(value, 0, 1);
      break;
    default:
      return;
  }
  set_led_color(index, false, r, g, b);
}

void Thymio2::set_led_color(size_t index, bool force, float r, float g, float b) {
  // printf("set led color %zu (%.2f %.2f %.2f)\n", index, r, g, b);
  if (index >= LED::COUNT) return;
  const LEDTexture & led_texture = led_textures[index];
  LED & led = leds[index];
  if (!force && !led.color.set_rgb(r, g, b)) return;
  // printf("push led color %zu (%.2f %.2f %.2f %.2f)\n",
  //        index, led.color.r, led.color.g, led.color.b, led.color.a);
  uint8_t * base_image;
  uint32_t * led_image = led_texture_images[led_texture.texture_index].ptr<uint32_t>();
  if (index < 3) {
    base_image = body_texture.ptr<uint8_t>();
  } else {
    base_image = texture.ptr<uint8_t>();
  }
  for (auto & a : led_texture.regions) {
    uint8_t * roi = (uint8_t *) malloc(a.h * a.w * 3);
    draw_rect(roi, base_image, led_image, TEXTURE_SIZE, a.x, a.y, a.w, a.h,
              led.color.r, led.color.g, led.color.b, led.color.a,
              index < 3 ? texture.ptr<uint8_t>() : nullptr);
    simWriteTexture(texture_id, 0, (const char *)roi, a.x, TEXTURE_SIZE - a.y - a.h, a.w, a.h, 0);
    free(roi);
  }
  for (auto & index : led_texture.overlapping_leds) {
    set_led_color(index, true);
  }
}

void Thymio2::update_sensing(float dt) {
  Robot::update_sensing(dt);
}

void Thymio2::update_actuation(float dt) {
  Robot::update_actuation(dt);
  behavior->do_step(dt);
  // reset r5
  r5 = false;
}

// to align z axis
static void get_vector_orientation(float * dp, float * q) {
  float nhs = dp[0] * dp[0] + dp[1] * dp[1];
  float n  = sqrt(nhs + dp[2] * dp[2]);
  float nh  = sqrt(nhs);
  // rot angle
  float a = acos(dp[2]/n);
  // rot axis = (0, 0, 1) x dp = (-dp[1], dp[0], 0)
  q[0] = -sin(a/2) * dp[1] / nh;
  q[1] = sin(a/2) * dp[0] / nh;
  q[2] = 0;
  q[3] = cos(a/2);
}

// TODO(Jerome) check params
static float prox_comm_sensor_response(
    float distance, float cos_emitter_angle, float cos_receiver_angle,
    float x0 = 2.0e-3,
    float c = 0.0917,
    float k_e = 18,
    float k_r = 9) {
  return (pow(abs(cos_emitter_angle), k_e) * pow(abs(cos_receiver_angle), k_r) * (c - x0 * x0) /
          (distance - x0) / (distance - x0));
}

static float prox_comm_response(float intensity,
                                float m = 4600.0,
                                float min_value = 1400) {
  float v = m / (1 / intensity + 1);
  if (v < min_value) v= 0.0;
  return v;
}

// NOTE(Jerome): My enki implementation differ a bit from what I documented:
// - I use 5 rays, not 3 so the max_emitter_angle is 30 degrees, not 15.
// - there is a factor cos(a_r)^k_r * cos(a_e)*k_e
// Difference between my enki and this implementation
// - enki: I sum over all sectors; here: I just use 1 ray, not sure if this is still calibrated

static bool check_emitter_receiver(simInt emitter_handle, simInt receiver_handle,
                                   simInt receiver_sensor,
                                   float & distance,
                                   float & cos_emitter_angle,
                                   float & cos_receiver_angle,
                                   float max_emitter_angle = 0.524,
                                   float max_receiver_angle = 0.644,
                                   float max_range = 0.48) {
  float position[3];
  simGetObjectPosition(receiver_handle, emitter_handle, position);
  distance = sqrt(position[0] * position[0] + position[1] * position[1] +
                  position[2] * position[2]);
  // printf("distance %.2f <? %.2f\n", distance, max_range);
  if (distance > max_range) return false;
  cos_emitter_angle = position[2] / distance;
  float emitter_angle = abs(acos(cos_emitter_angle));
  // printf("emitter angle %.2f <? %.2f\n", emitter_angle, max_emitter_angle);
  if (emitter_angle > max_emitter_angle) return false;
  float emitter_position[3];
  simGetObjectPosition(emitter_handle, receiver_handle, emitter_position);
  cos_receiver_angle = emitter_position[2] / distance;
  float receiver_angle = abs(acos(cos_receiver_angle));
  // printf("receiver angle %.2f <? %.2f\n", receiver_angle, max_receiver_angle);
  if (receiver_angle > max_receiver_angle) return false;
  float q[4];
  get_vector_orientation(emitter_position, q);
  // printf("Set orientation of ray %d to %.2f %.2f %.2f %.2f\n",
  //        receiver_sensor, q[0], q[1], q[2], q[3]);
  simSetObjectQuaternion(receiver_sensor, receiver_handle, q);
  simInt detectedObjectHandle = -1;
  float detectedPoint[4];
  simInt r = simCheckProximitySensorEx(receiver_sensor, sim_handle_all, 5, distance - 1e-3, 0.0,
                                       detectedPoint, &detectedObjectHandle, nullptr);
  // printf("Collision? %d: %d (%d) at %.2f (%.2f)\n",
  // receiver_sensor, r, detectedObjectHandle, detectedPoint[3], distance - 1e-3);
  return r == 0;
}

void ProximityComm::update_sensing(const std::array<simInt, 7> & tx_handles, simInt value) {
  // printf("update_sensing %d\n", value);
  ProxCommMsg msg;
  int i = 0;
  bool received = false;
  for (size_t i = 0; i < 7; i++) {
    /* code */
    simInt receiver = emitter_handles[i];
    simInt sensor = sensor_handles[i];
    float intensity = 0.0;
    // printf("receiver %d with sensor %d\n", receiver, sensor);
    for (const simInt emitter : tx_handles) {
      float distance, cos_e, cos_r;
      // printf("emitter %d\n", emitter);
      if (check_emitter_receiver(emitter, receiver, sensor, distance, cos_e, cos_r)) {
        intensity += prox_comm_sensor_response(distance, cos_e, cos_r);
      }
    }
    msg.intensities[i] = prox_comm_response(intensity);
    if (msg.intensities[i]) {
      received = true;
      msg.payloads[i] = value;
      msg.rx = value;
    } else {
      msg.payloads[i] = 0;
    }
  }
  if (received) {
    rx_buffer.push_back(msg);
  }
}

}  // namespace CS
