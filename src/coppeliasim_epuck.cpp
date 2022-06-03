#include "coppeliasim_epuck.h"
#include "logging.h"

#include <math.h>

namespace CS {

void Camera::update_sensing(float dt) {
  if (!active || handle <= 0) return;
  simHandleVisionSensor(handle, nullptr, nullptr);
  simInt width = 0;
  simInt height = 0;
  simUChar *buffer = simGetVisionSensorCharImage(handle, &width, &height);
  if (width != image_width || height != image_height) {
    log_error("Wrong image size %d x %d", width, height);
    return;
  }
  unsigned size = width * height * 3;
  image = std::vector<uint8_t>(buffer, buffer + size);
  simReleaseBuffer((const simChar *)buffer);
}

void Gyroscope::update_sensing(float dt) {
  if (!active || handle <= 0) return;
  simFloat w[3];
  // get angular velocity in absolute coordinates
  simGetObjectVelocity(handle+sim_handleflag_axis, nullptr, w);
  simFloat t[12];
  simGetObjectMatrix(handle, -1, t);
  // just rotation
  t[3] = t[7] = t[11] = 0.0;
  simInvertMatrix(t);
  // get angular velocity in body frame
  simTransformVector(t, w);
  std::copy(w, w+ 3, values);
}

// TODO(Jerome): calibrate
static const float max_value = 3731.0;
static const float min_value = 0.0;
static const float x0 = 0.0;
static const float lambda = 0.083;
static std::array<std::string, 2> wheel_prefixes = {"/Left", "/Right"};
static std::array<std::string, 8> proximity_names = {
    "0", "1", "2", "3", "4", "5", "6", "7"};
static std::array<std::string, 3> ground_names = {"Left", "Center", "Right"};

EPuck::EPuck(simInt handle_) :
  Robot(handle_), front_led(false), body_led(false),
  mic_intensity({0, 0, 0}), battery_voltage(4.0), selector(0), rc(0) {
  simChar * alias = simGetObjectAlias(handle, 2);
  log_info("Initializing EPuck with handle %d (%s)", handle, alias);
  for (const auto & wheel_prefix : wheel_prefixes) {
    std::string wheel_path = std::string(alias) + wheel_prefix + "Motor";
    simInt wheel_handle = simGetObject(wheel_path.c_str(), -1, -1, 0);
    wheels.push_back(Wheel(0.0211745, wheel_handle));
  }
  for (const auto & proximity_name : proximity_names) {
    std::string prox_path = std::string(alias)+"/Proximity_" + proximity_name;
    simInt prox_handle = simGetObject(prox_path.c_str(), -1, -1, 0);
    proximity_sensors.push_back(ProximitySensor(prox_handle, min_value, max_value, x0, lambda));
  }
  // for (const auto & ground_name : ground_names) {
  //   std::string ground_path = std::string(alias)+"/Ground" + ground_name;
  //   simInt ground_handle = simGetObject(ground_path.c_str(), -1, -1, 0);
  //   ground_sensors.push_back(GroundSensor(ground_handle));
  // }
  std::string acc_path = std::string(alias)+"/Accelerometer";
  simInt acc_handle = simGetObject(acc_path.c_str(), -1, -1, 0);
  accelerometer = Accelerometer(acc_handle);

  std::string camera_path = std::string(alias)+"/Camera";
  camera = Camera(simGetObject(camera_path.c_str(), -1, -1, 0));

  gyroscope = Gyroscope(simGetObject((std::string(alias)+"/Gyroscope").c_str(), -1, -1, 0));

  ring_handle = simGetObject((std::string(alias)+"/Ring").c_str(), -1, -1, 0);
  body_handle = simGetObject((std::string(alias)+"/Body").c_str(), -1, -1, 0);
  rest_handle = simGetObject((std::string(alias)+"/Rest").c_str(), -1, -1, 0);
  leds = LEDRing(ring_handle);
  simReleaseBuffer(alias);
  reset();
}

EPuck::~EPuck() {
  set_body_led(false);
  set_front_led(false);
  leds.reset(false);
}

void EPuck::reset() {
  Robot::reset();
  leds.reset(true);
  set_body_led(false);
  set_front_led(false);
  log_info("Reset e-puck");
}

void EPuck::update_sensing(float dt) {
  Robot::update_sensing(dt);
  camera.update_sensing(dt);
  gyroscope.update_sensing(dt);
}

void EPuck::update_actuation(float dt) {
  Robot::update_actuation(dt);
}

void EPuck::set_body_led(bool value) {
  // log_debug("set_body_led %d\n", value);
  if (value != body_led) {
    body_led = value;
    simFloat color[3] = {0.0, 0.27f * value, 0.0};
    simInt r = simSetShapeColor(body_handle, nullptr, sim_colorcomponent_emission, color);
    color[1] *= 0.8;
    simSetShapeColor(ring_handle, nullptr, sim_colorcomponent_emission, color);
    color[1] *= 0.8;
    simSetShapeColor(rest_handle, nullptr, sim_colorcomponent_emission, color);
  }
}

void EPuck::set_front_led(bool value) {
  if (value != front_led) {
    // TODO(Jerome): complete
    front_led = value;
  }
}

LEDRing::LEDRing(simInt shape_handle_) : shape_handle(shape_handle_) {
  if (shape_handle <= 0) return;
  texture_id = simGetShapeTextureId(shape_handle);
  log_info("loading led texture");
  texture = cv::imread(TEXTURE_DIR "/epuck.png");
  simInt64 uid = simGetObjectUid(shape_handle);
  // HACK(Jerome): One pixel should be specific to each robot,
  // else coppeliaSim will link them when it save the scene
  texture.data[0] = (uint8_t) (uid & 0xFF);
  texture.data[1] = (uint8_t) ((uid >> 8) & 0xFF);
  texture.data[2] = (uint8_t) ((uid >> 16) & 0xFF);
  log_info("loaded led texture of size (%d, %d)", texture.size().width, texture.size().height);
  reset(true);
  for (auto position : positions) {
    leds.emplace_back(position, texture);
  }
}

void LEDRing::reset(bool reload) {
  if (shape_handle <= 0) return;
  for (auto & led : leds) {
    led.value = false;
  }
  cv::Mat t;
  cv::cvtColor(texture, t, cv::COLOR_BGR2RGB);
  cv::flip(t, t, 0);
  log_info("resetting led texture (%d)", reload);
  if (reload) {
    SShapeVizInfo info;
    simInt r = simGetShapeViz(shape_handle, 0, &info);
    texture_id = simApplyTexture(
        shape_handle, info.textureCoords, info.indicesSize * 2,
        (const simUChar *)t.ptr(), info.textureRes, 1);
    log_debug("Reset texture for handle %d -> texture_id %d", shape_handle, texture_id);
    simReleaseBuffer((const char *)info.indices);
    simReleaseBuffer((const char *)info.normals);
    simReleaseBuffer((const char *)info.texture);
    simReleaseBuffer((const char *)info.textureCoords);
  } else {
    simWriteTexture(texture_id, 0, (const char *)t.ptr(), 0, 0, texture_size, texture_size, 0);
  }
  log_info("reset led texture");
}

void LEDRing::set_value(size_t index, bool v) {
  if (shape_handle <= 0) return;
  if (index > leds.size()) return;
  if (index == leds.size()) {
    for (size_t i = 0; i < leds.size(); i++) {
      set_value(i, v);
    }
    return;
  }
  if (v != leds[index].value) {
    leds[index].value = v;
    leds[index].push(texture_id, texture_size);
  }
}

static void draw_blob(uint8_t* target, const uint8_t* base, int width,
                      int x0, int y0, int size_x, int size_y,
                      float a, float rx, float ry,
                      uint8_t colorR, uint8_t colorG, uint8_t colorB) {
  uint8_t *t = target;
  for (int y = size_y-1; y >= 0; y--) {
    float dy = (size_y/2 - y) / ry;
    for (int x = 0; x < size_x; x++) {
      const size_t index(x + x0 + width * (y + y0));
      const uint32_t baseR(base[3 * index+2]);
      const uint32_t baseG(base[3 * index+1]);
      const uint32_t baseB(base[3 * index]);
      float dx = (size_x/2 - x) / rx;
      const uint32_t sourceA = std::clamp<uint32_t>(a * exp(-dx*dx - dy*dy) * 255, 0, 255);
      const uint32_t oneMSrcA(255-sourceA);
      *t++ = ((baseR * oneMSrcA + colorR * sourceA) >> 8);
      *t++ = ((baseG * oneMSrcA + colorG * sourceA) >> 8);
      *t++ = ((baseB * oneMSrcA + colorB * sourceA) >> 8);
    }
  }
}

LEDRing::LED::LED(int position, const cv::Mat & texture) : value(false), position(position) {
  log_info("Creating LED at %d", position);
  cv::Mat patch = texture(cv::Rect(position, y, patch_width, patch_height));
  cv::flip(patch, off_texture, 0);
  cv::cvtColor(off_texture, off_texture, cv::COLOR_BGR2RGB);
  on_texture = patch.clone();
  draw_blob(on_texture.ptr<uint8_t>(), texture.ptr<uint8_t>(), texture.size().width, position,
            y, patch_width, patch_height, a, size_x, size_y, 255, 0, 0);
  log_info("Created LED");
}

void LEDRing::LED::push(simInt texture_id, int texture_size) {
  const char * patch = (const char *) (value ? on_texture : off_texture).ptr<uint8_t>();
  simWriteTexture(texture_id, 0, patch, position, texture_size - y - patch_height, patch_width,
                  patch_height, 0);
}

// void EPuck::set_ring_led(size_t index, bool value) {
//   if (index > 7) return;
//   int x = 0;
//   int ring_led_y = 266;
//   int ring_led_size = 40;
//   uint8_t * base_image = ring_texture.ptr<uint8_t>();
//   uint8_t * roi = (uint8_t *) malloc(ring_led_size * ring_led_size * 3);
//   // int x = ring_led_xs[i];
//   draw_blob(roi, base_image, TEXTURE_SIZE, x, ring_led_y, ring_led_size, 2.0, 10.0, 15.0,
//             255 * value, 0, 0);
//   simWriteTexture(ring_texture_id, 0, (const char *)roi, x,
//                   TEXTURE_SIZE - ring_led_y - ring_led_size, ring_led_size, ring_led_size, 0);
//
//   free(roi);
// }


}  // namespace CS
