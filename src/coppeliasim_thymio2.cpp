#include "coppeliasim_thymio2.h"
#include "logging.h"

#define TEXTURE_SIZE 1024

static cv::Mat body_texture;
static std::array<cv::Mat, 3> led_texture_images;
static bool loaded_textures = false;

namespace CS {

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
             LED::LEFT_RED, LED::LEFT_BLUE, LED::RIGHT_BLUE, LED::RIGHT_RED}},
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
  LEDTexture{{{775, 426, 50, 50}}, LED_TEXTURE}
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

Thymio2::Thymio2(simInt handle_) : handle(handle_) {
  simChar * alias = simGetObjectAlias(handle, 2);
  std::string body_path = std::string(alias)+"/Body";
  body_handle = simGetObject(body_path.c_str(), -1, -1, 0);
  texture_id = simGetShapeTextureId(body_handle);
  log_info("Initializing Thymio2 with handle %d and texture_id %d", handle, texture_id);
  for (size_t i = 0; i < wheels.size(); i++) {
    std::string wheel_path = std::string(alias) + wheel_prefixes[i] + "Motor";
    simInt wheel_handle = simGetObject(wheel_path.c_str(), -1, -1, 0);
    wheels[i] = Wheel(wheel_handle);
  }
  for (size_t i = 0; i < proximity_sensors.size(); i++) {
    std::string prox_path = std::string(alias)+"/Proximity" + proximity_names[i];
    simInt prox_handle = simGetObject(prox_path.c_str(), -1, -1, 0);
    proximity_sensors[i] = ProximitySensor(prox_handle);
    prox_comm.emitter_handles[i] = prox_handle;
    prox_comm.sensor_handles[i] = simGetObject((prox_path + "/Comm").c_str(), -1, -1, 0);
  }
  for (size_t i = 0; i < ground_sensors.size(); i++) {
    std::string ground_path = std::string(alias)+"/Ground" + ground_names[i];
    simInt ground_handle = simGetObject(ground_path.c_str(), -1, -1, 0);
    ground_sensors[i] = GroundSensor(ground_handle);
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
  for (auto & led : leds) {
    led.color.a = 0;
  }
  reset_texture(true);
  set_target_speed(Wheel::LEFT, 0.0);
  set_target_speed(Wheel::RIGHT, 0.0);
}

void Thymio2::set_target_speed(size_t index, float speed) {
  if (index < wheels.size()) {
    wheels[index].set_target_speed(speed);
  }
}

float Thymio2::get_target_speed(size_t index) {
  if (index < wheels.size()) {
    return wheels[index].get_target_speed();
  }
  return 0.0;
}

void Thymio2::set_led_intensity(size_t index, float intensity) {
  if (index >= LED::COUNT) return;
  LED & led = leds[index];
  if (led.color.set_a(std::clamp(intensity, 0.0f, 1.0f))) {
    set_led_color(index, true);
  }
}

void Thymio2::set_led_color(size_t index, bool force,
                                       float r, float g, float b) {
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
  for (auto & wheel : wheels) {
    wheel.update_sensing(dt);
  }
  for (auto & prox : proximity_sensors) {
    if (prox.active) prox.update_sensing(dt);
  }
  for (auto & ground : ground_sensors) {
    if (ground.active) ground.update_sensing(dt);
  }
  if (accelerometer.active) accelerometer.update_sensing(dt);
}

void Thymio2::update_actuation(float dt) {}

// TODO(Jerome): should be in two different callbacks/messages
void Thymio2::do_step(float dt) {
  update_sensing(dt);
  update_actuation(dt);
}



void Wheel::set_target_speed(float speed) {
  float value = speed / radius;
  if (value != nominal_angular_target_speed) {
    simSetJointTargetVelocity(handle, value);
    nominal_angular_target_speed = value;
  }
}

void Wheel::update_sensing(float dt) {
  simInt r = simGetJointVelocity(handle, &angular_speed);
}

static float ir_response(
    float intensity, float max_value, float min_value) {
  if (intensity <= 0) {
    return 0;
  }
  float v = max_value / (1 + 1/intensity);
  if (v < min_value) {
    return 0;
  }
  return v;
}

static float ir_reflected_intensity(
    float distance, float normal, float reflectivity,
    float x0, float lambda) {
  float r = (lambda - x0) / (distance - x0);
  return r * r * abs(normal) * reflectivity;
}

static float ir_reflectivity(float r, float g, float b, bool only_red = false,
                             float beta = 0.11, float v0 = 0.44) {
  float v;
  if (only_red) {
    v = 0.8 * r + 0.2;
  } else {
    v = (r + g + b) / 3;
  }
  return 1. / (1. + exp(-(v - v0) / beta));
}

static float ir(float distance, float normal, float r, float g, float b,
                float max_value, float min_value, float x0, float lambda,
                bool only_red = false, float beta = 0.11, float v0 = 0.44) {
  return ir_response(
      ir_reflected_intensity(
          distance, normal,
          ir_reflectivity(r, g, b, only_red, beta, v0), x0, lambda),
        max_value, min_value);
}

// From enki: it ignores the distance (as enki is 2D)
// TODO(Jerome): consider the distance too
// inline double _sigm(float x, float s) {
//   return 1. / (1. + exp(-x * s));
// }

// static float proximity_response(
//     float distance, float normal, float intensity,
//     float x0 = 0.0003, float c = 0.0073, float m = 4505, float min_value = 1000) {
//   float r = distance - x0;
//   float d = 1 + r * r / (c - x0 * x0) / abs(normal) / intensity;
//   if (d < 1) {
//     return m;
//   }
//   float v = m / d;
//   if (v < min_value) {
//     return 0;
//   }
//   return v;
// }

void ProximitySensor::update_sensing(float dt) {
  // TODO(Jerome): check if we should use max angle and which detection mode
  // (now No and front face, accurate)
  // https://www.coppeliarobotics.com/helpFiles/en/regularApi/simCheckProximitySensorEx.htm
  simFloat detectedPoint[4];
  simInt detectedObjectHandle = 0;
  simFloat surfaceNormalVector[3];

  simInt r = simCheckProximitySensorEx(
      handle, sim_handle_all, 1, 0.28, 0, detectedPoint, &detectedObjectHandle,
      surfaceNormalVector);
  // printf("Checked %d -> %d\n", handle);
  if (r > 0) {
    detected = true;
    // get diffusion color of detected object
    simFloat rgbData[3];
    r = simGetObjectColor(detectedObjectHandle, 0, sim_colorcomponent_ambient_diffuse, rgbData);
    // float intensity = std::max({rgbData[0], rgbData[1], rgbData[2]});
    // value = proximity_response(detectedPoint[3], surfaceNormalVector[2], intensity);
    //
    value = ir(detectedPoint[3], surfaceNormalVector[2], rgbData[0], rgbData[1], rgbData[2],
               max_value, min_value, x0, lambda, only_red);
    // printf("Detected: %.2f %.2f %.2f -> %.2f\n", detectedPoint[3], surfaceNormalVector[2],
           // intensity, value);
  } else {
    // printf("not Detected\n");
    detected = false;
  }
}

GroundSensor::GroundSensor(simInt handle_) :
  handle(handle_), active(true), only_red(false), use_vision(false) {
  if (handle >= 0) {
    simChar * alias = simGetObjectAlias(handle, 2);
    std::string path = std::string(alias);
    simReleaseBuffer(alias);
    vision_handle = simGetObject((path + "/Vision").c_str(), -1, -1, 0);
  }
}

// static float ground_response(
//     float distance, float normal, float value,
//     float cFactor = 0.44, float sFactor = 9, float mFactor = 884, float aFactor = 60,
//     float x0 = 0.0084, float c = 0.000187, float m = 1032.0) {
//
//     // v = _sigm(intensity - cFactor, sFactor) * mFactor + aFactor;
//     float r = _sigm(value - cFactor, sFactor);
//     float d = distance - x0;
//     float v = 1.0 + (d * d) / (c - x0 * x0) / abs(normal);
//     return std::clamp(r * m / v, 0.0f, 1023.0f);
// }

void GroundSensor::update_sensing(float dt) {
  simFloat detectedPoint[4];
  simInt detectedObjectHandle = 0;
  simFloat surfaceNormalVector[3];
  simInt r = simCheckProximitySensorEx(
      handle, sim_handle_all, 1, 0.1, 0, detectedPoint, &detectedObjectHandle,
      surfaceNormalVector);
  // printf("GroundSensor::update_sensing r %d %d %.3f\n", detectedObjectHandle, detectedPoint[3]);
  // printf("GS update_sensing\n");
  if (r > 0) {
    float rgb[3];
    // printf("Detection %d\n", detectedObjectHandle);
    // bool has_texture = (simGetShapeTextureId(detectedObjectHandle) != -1);
    if (use_vision) {
      simFloat* auxValues = nullptr;
      simInt* auxValuesCount = nullptr;
      // const simFloat * rgbData = simCheckVisionSensorEx(vision_handle, sim_handle_all, true);
      // simInt r1 = simCheckVisionSensor(vision_handle, detectedObjectHandle,
      //                                  &auxValues, &auxValuesCount);
      simHandleVisionSensor(vision_handle, &auxValues, &auxValuesCount);
      if ((auxValuesCount[0] > 0) && (auxValuesCount[1] >= 15)) {
        rgb[0] = auxValues[11];
        rgb[1] = auxValues[12];
        rgb[2] = auxValues[13];
        // intensity = std::max({auxValues[11], auxValues[12], auxValues[13]});
      } else {
        // printf("ERORR: no detection by %d\n", vision_handle);
        rgb[0] = 0;
        rgb[1] = 0;
        rgb[2] = 0;
      }
      simReleaseBuffer((char*)auxValues);
      simReleaseBuffer((char*)auxValuesCount);
      // return;
      // if (rgbData) {
      //   simInt re[2];
      //   simGetVisionSensorResolution(vision_handle, re);
      //   printf("%.3f %.3f %3.f (%d, %d, %d)\n",
      //   rgbData[0], rgbData[1], rgbData[2], re[0], re[1]);
      //   intensity = std::max({rgbData[0], rgbData[1], rgbData[2]});
      // } else {
      //   intensity = 0.0;
      // }
      // simReleaseBuffer((const simChar *) rgbData);
    } else {
      // printf("no texture\n");
      simGetObjectColor(detectedObjectHandle, 0, sim_colorcomponent_ambient_diffuse, rgb);
      // intensity = std::max({rgbData[0], rgbData[1], rgbData[2]});
    }
    // printf("intensity %.2f\n", intensity);
    // reflected_light = ground_response(detectedPoint[3], surfaceNormalVector[2], intensity);
    reflected_light = ir(detectedPoint[3], surfaceNormalVector[2], rgb[0], rgb[1], rgb[2],
                         max_value, min_value, x0, lambda, only_red);
  } else {
    // printf("No detection\n");
    // reflected_light = 0;
    reflected_light = min_value;
  }
}

Accelerometer::Accelerometer(int handle_) : handle(handle_), active(true) {
  if (handle >= 0) {
    simChar * alias = simGetObjectAlias(handle, 2);
    std::string path = std::string(alias);
    simReleaseBuffer(alias);
    simInt massObject = simGetObject((path + "/forceSensor/mass").c_str(), -1, -1, 0);
    sensor = simGetObject((path + "/forceSensor").c_str(), -1, -1, 0);
    simInt r = simGetObjectFloatParam(massObject, sim_shapefloatparam_mass, &mass);
    if (r != 1) {
      log_error("Error %d getting mass of object %d", r, massObject);
    }
  }
}

void Accelerometer::update_sensing(float dt) {
    float force[3];
    simInt result = simReadForceSensor(sensor, force, nullptr);
    if (result > 0) {
      // printf("Acc: %.1e %1.e %.1e %.1e\n", force[0], force[1], force[2], mass);
      for (size_t i = 0; i < 3; i++) {
        values[i] = -force[i] / mass;
      }
    } else {
      // printf("No acc\n");
      for (size_t i = 0; i < 3; i++) {
        values[i] = 0;
      }
    }
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
