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
  Robot(handle_), mic_intensity({0, 0, 0}), battery_voltage(4.0), selector(0), rc(0) {
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
  simReleaseBuffer(alias);
  reset();
}

EPuck::~EPuck() {

}

void EPuck::reset() {
  Robot::reset();
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

}  // namespace CS
