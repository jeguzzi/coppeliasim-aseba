#include "coppeliasim_thymio2.h"

#define TEXTURE_SIZE 1024

static cv::Mat body_texture;
static std::array<cv::Mat, 3> led_texture_images;
static bool loaded_textures = false;

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

std::array<LEDTexture, LED_COUNT> led_textures = {
  LEDTexture{{{130, 535, 350, 488}, {485, 630, 500, 100},
              {480, 846, 265, 50}, {790, 160, 50, 265}
            }, TOP_TEXTURE,
            {BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT,
             RING_2, RING_3, RING_4, RING_5, RING_6,
             IR_BACK_0, IR_BACK_1,
             LEFT_RED, LEFT_BLUE, RIGHT_BLUE, RIGHT_RED}},
  LEDTexture{{{563, 38, 117, 301}, {651, 731, 210, 113}}, BOTTOM_TEXTURE,
             {IR_FRONT_0, IR_FRONT_1, IR_FRONT_2, IR_FRONT_3,
              LEFT_RED, LEFT_BLUE, RIGHT_BLUE, RIGHT_RED}},
  LEDTexture{{{565, 344, 224, 192}}, BOTTOM_TEXTURE,
             {IR_FRONT_2, IR_FRONT_3, IR_FRONT_4, IR_FRONT_5,
              LEFT_RED, LEFT_BLUE, RIGHT_BLUE, RIGHT_RED}},
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

void CoppeliaSimThymio2::reset_texture() {
  load_textures();
  texture = cv::Mat(TEXTURE_SIZE, TEXTURE_SIZE, CV_8UC3);
  cv::cvtColor(body_texture, texture, cv::COLOR_BGR2RGB);
  cv::Mat m = cv::Mat(TEXTURE_SIZE, TEXTURE_SIZE, CV_8UC3);
  cv::flip(texture, m, 0);
  simWriteTexture(texture_id, 0, (const char *)m.ptr(), 0, 0, TEXTURE_SIZE, TEXTURE_SIZE, 0);
}

void CoppeliaSimThymio2::reset() {
  for (auto & led : leds) {
    led.color.a = 0;
  }
  reset_texture();
  set_target_speed(LEFT_WHEEL, 0.0);
  set_target_speed(RIGHT_WHEEL, 0.0);
}

static std::array<std::string, 2> wheel_prefixes = {"/Left", "/Right"};
static std::array<std::string, 7> proximity_names = {
    "Left", "CenterLeft", "Center", "CenterRight", "Right", "RearLeft", "RearLeft"};
static std::array<std::string, 2> ground_names = {"Left", "Right"};

CoppeliaSimThymio2::CoppeliaSimThymio2(simInt handle) {
  simChar * alias = simGetObjectAlias(handle, 2);
  std::string body_path = std::string(alias)+"/Body";
  simInt body_handle = simGetObject(body_path.c_str(), -1, -1, 0);
  texture_id = simGetShapeTextureId(body_handle);
  for (size_t i = 0; i < wheels.size(); i++) {
    std::string wheel_path = std::string(alias) + wheel_prefixes[i] + "Motor";
    simInt wheel_handle = simGetObject(wheel_path.c_str(), -1, -1, 0);
    wheels[i] = Wheel(wheel_handle);
  }
  for (size_t i = 0; i < proximity_sensors.size(); i++) {
    std::string prox_path = std::string(alias)+"/Proximity" + proximity_names[i];
    simInt prox_handle = simGetObject(prox_path.c_str(), -1, -1, 0);
    proximity_sensors[i] = ProximitySensor(prox_handle);
  }
  for (size_t i = 0; i < ground_sensors.size(); i++) {
    std::string ground_path = std::string(alias)+"/Ground" + ground_names[i];
    simInt ground_handle = simGetObject(ground_path.c_str(), -1, -1, 0);
    ground_sensors[i] = GroundSensor(ground_handle);
  }
  std::string acc_path = std::string(alias)+"/Accelerometer";
  simInt acc_handle = simGetObject(acc_path.c_str(), -1, -1, 0);
  accelerometer = Accelerometer(acc_handle);
  simReleaseBuffer(alias);

  for (size_t i = BUTTON_UP; i <= BUTTON_RIGHT; i++) {
    leds[i].color = Color(1, 0, 0);
  }
  for (size_t i = RING_0; i <= RING_7; i++) {
    leds[i].color = Color(1, 0.5, 0);
  }
  for (size_t i = IR_FRONT_0; i <= IR_BACK_1; i++) {
    leds[i].color = Color(1, 0, 0);
  }
  leds[RIGHT_RED].color = Color(1, 0, 0);
  leds[LEFT_BLUE].color = Color(0, 1, 1);
  leds[RIGHT_BLUE].color = Color(0, 1, 1);
  reset();
}

CoppeliaSimThymio2::~CoppeliaSimThymio2() {
  reset_texture();
}

void CoppeliaSimThymio2::set_target_speed(size_t index, float speed) {
  if (index < wheels.size()) {
    wheels[index].set_target_speed(speed);
  }
}

void CoppeliaSimThymio2::set_led_intensity(size_t index, float intensity) {
  if (index >= LED_COUNT) return;
  LED & led = leds[index];
  if (led.color.set_a(std::clamp(intensity, 0.0f, 1.0f))) {
    set_led_color(index, true);
  }
}

void CoppeliaSimThymio2::set_led_color(size_t index, bool force,
                                       float r, float g, float b) {
  // printf("set led color %zu (%.2f %.2f %.2f)\n", index, r, g, b);
  if (index >= LED_COUNT) return;
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

void CoppeliaSimThymio2::update_sensing(float dt) {
  for (auto & wheel : wheels) {
    wheel.update_sensing(dt);
  }
  for (auto & prox : proximity_sensors) {
    prox.update_sensing(dt);
  }
  for (auto & ground : ground_sensors) {
    ground.update_sensing(dt);
  }
  accelerometer.update_sensing(dt);
}

void CoppeliaSimThymio2::update_actuation(float dt) {}

// TODO(Jerome): should be in two different callbacks/messages
void CoppeliaSimThymio2::do_step(float dt) {
  update_sensing(dt);
  update_actuation(dt);
}



void CoppeliaSimThymio2::Wheel::set_target_speed(float speed) {
  float value = speed / radius;
  if (value != nominal_angular_target_speed) {
    simSetJointTargetVelocity(handle, value);
    nominal_angular_target_speed = value;
  }
}

void CoppeliaSimThymio2::Wheel::update_sensing(float dt) {
  simInt r = simGetJointVelocity(handle, &angular_speed);
}

static float proximity_response(
    float distance, float normal, float intensity,
    float x0 = 0.0003, float c = 0.0073, float m = 4505, float min_value = 1000) {
  float r = distance - x0;
  float d = 1 + r * r / (c - x0 * x0) * abs(normal) * intensity;
  if (d < 1) {
    return m;
  }
  float v = m / d;
  if (v < min_value) {
    return 0;
  }
  return v;
}

void CoppeliaSimThymio2::ProximitySensor::update_sensing(float dt) {
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
    float intensity = std::max({rgbData[0], rgbData[1], rgbData[2]});
    value = proximity_response(detectedPoint[3], surfaceNormalVector[2], intensity);
    // printf("Detected: %.2f %.2f %.2f -> %.2f\n", detectedPoint[3], surfaceNormalVector[2],
           // intensity, value);
  } else {
    // printf("not Detected\n");
    detected = false;
  }
}

CoppeliaSimThymio2::GroundSensor::GroundSensor(simInt handle_) :
  handle(handle_) {
  if (handle >= 0) {
    simChar * alias = simGetObjectAlias(handle, 2);
    std::string path = std::string(alias);
    simReleaseBuffer(alias);
    vision_handle = simGetObject((path + "/Vision").c_str(), -1, -1, 0);
  }
}

// From enki: it ignores the distance (as enki is 2D)
// TODO(Jerome): consider the distance too
inline double _sigm(float x, float s) {
  return 1. / (1. + exp(-x * s));
}

static float ground_response(
    float distance, float intensity,
    float cFactor = 0.44, float sFactor = 9, float mFactor = 884, float aFactor = 60) {
    return _sigm(intensity - cFactor, sFactor) * mFactor + aFactor;
}

void CoppeliaSimThymio2::GroundSensor::update_sensing(float dt) {
  simFloat detectedPoint[4];
  simInt detectedObjectHandle = 0;
  simFloat surfaceNormalVector[3];

  simInt r = simCheckProximitySensorEx(
      handle, sim_handle_all, 1, 0.1, 0, detectedPoint, &detectedObjectHandle,
      surfaceNormalVector);
  // printf("GS update_sensing\n");
  if (r > 0) {
    // printf("Detection %d\n", detectedObjectHandle);
    // bool has_texture = (simGetShapeTextureId(detectedObjectHandle) != -1);
    bool has_texture = true;
    float intensity;
    if (has_texture) {
      simFloat* auxValues = nullptr;
      simInt* auxValuesCount = nullptr;
      // const simFloat * rgbData = simCheckVisionSensorEx(vision_handle, sim_handle_all, true);
      simInt r1 = simCheckVisionSensor(vision_handle, sim_handle_all, &auxValues, &auxValuesCount);
      // HACK(Jerome): seems to always return 0
      if ((auxValuesCount[0] > 0) || (auxValuesCount[1] >= 15)) {
        intensity = std::max({auxValues[11], auxValues[12], auxValues[13]});
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
      simFloat rgbData[3];
      simGetObjectColor(detectedObjectHandle, 0, sim_colorcomponent_ambient_diffuse, rgbData);
      intensity = std::max({rgbData[0], rgbData[1], rgbData[2]});
    }
    // printf("intensity %.2f\n", intensity);
    reflected_light = ground_response(detectedPoint[3], intensity);
  } else {
    // printf("No detection\n");
    reflected_light = 0;
  }
}

CoppeliaSimThymio2::Accelerometer::Accelerometer(int handle_) : handle(handle_) {
  if (handle >= 0) {
    simChar * alias = simGetObjectAlias(handle, 2);
    std::string path = std::string(alias);
    simReleaseBuffer(alias);
    simInt massObject = simGetObject((path + "/forceSensor/mass").c_str(), -1, -1, 0);
    sensor = simGetObject((path + "/forceSensor").c_str(), -1, -1, 0);
    simInt r = simGetObjectFloatParam(massObject, sim_shapefloatparam_mass, &mass);
    if (r != 1) {
      printf("Error getting object %d mass\n", massObject);
    }
  }
}

void CoppeliaSimThymio2::Accelerometer::update_sensing(float dt) {
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
