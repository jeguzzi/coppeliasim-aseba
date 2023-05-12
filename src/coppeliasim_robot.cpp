#include "coppeliasim_robot.h"
#include "logging.h"

#include <math.h>

#define G 9.81f

// normalize between -pi and pi
static float normalize_angle(float angle) {
  angle = fmod(angle, 2 * M_PI);
  if (angle > M_PI) {
    return angle - 2 * M_PI;
  }
  if (angle < -M_PI) {
    return angle + 2 * M_PI;
  }
  return angle;
}

namespace CS {

Robot::Robot(int handle_) : handle(handle_), wheels(), proximity_sensors(), ground_sensors() { }

Robot::~Robot() { }


void Robot::reset() {
  for (auto & wheel : wheels) {
    wheel.set_target_speed(0.0f);
    wheel.first = true;
  }
}

void Robot::set_target_speed(size_t index, float speed) {
  if (index < wheels.size()) {
    wheels[index].set_target_speed(speed);
  }
}

float Robot::get_target_speed(size_t index) {
  if (index < wheels.size()) {
    return wheels[index].get_target_speed();
  }
  return 0.0;
}

void Robot::update_sensing(float dt) {
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

void Robot::update_actuation(float dt) { }

void Robot::do_step(float dt) {
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
  int r = simGetJointVelocity(handle, &angular_speed);
  simFloat new_angle;
  r = simGetJointPosition(handle, &new_angle);
  if (first) {
    odometry = 0.0;
    first = false;
  } else {
    odometry += normalize_angle(new_angle - angle);
  }
  angle = new_angle;
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
  int detectedObjectHandle = 0;
  simFloat surfaceNormalVector[3];

  int r = simCheckProximitySensorEx(
      handle, sim_handle_all, 1, 0.28, 0, detectedPoint, &detectedObjectHandle,
      surfaceNormalVector);
  // printf("Checked %d -> %d\n", handle);
  if (r > 0) {
    detected = true;
    // get diffusion color of detected object
    float rgbData[3];
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

GroundSensor::GroundSensor(int handle_) :
  handle(handle_), active(true), only_red(false), use_vision(false) {
  if (handle >= 0) {
    char * alias = simGetObjectAlias(handle, 2);
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
  int detectedObjectHandle = 0;
  simFloat surfaceNormalVector[3];
  int r = simCheckProximitySensorEx(
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
      int* auxValuesCount = nullptr;
      // const simFloat * rgbData = simCheckVisionSensorEx(vision_handle, sim_handle_all, true);
      // int r1 = simCheckVisionSensor(vision_handle, detectedObjectHandle,
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
      //   int re[2];
      //   simGetVisionSensorResolution(vision_handle, re);
      //   printf("%.3f %.3f %3.f (%d, %d, %d)\n",
      //   rgbData[0], rgbData[1], rgbData[2], re[0], re[1]);
      //   intensity = std::max({rgbData[0], rgbData[1], rgbData[2]});
      // } else {
      //   intensity = 0.0;
      // }
      // simReleaseBuffer((const char *) rgbData);
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
    char * alias = simGetObjectAlias(handle, 2);
    std::string path = std::string(alias);
    simReleaseBuffer(alias);
    int massObject = simGetObject((path + "/forceSensor/mass").c_str(), -1, -1, 0);
    sensor = simGetObject((path + "/forceSensor").c_str(), -1, -1, 0);
    int r = simGetObjectFloatParam(massObject, sim_shapefloatparam_mass, &mass);
    if (r != 1) {
      log_error("Error %d getting mass of object %d", r, massObject);
    }
  }
}

void Accelerometer::update_sensing(float dt) {
    simFloat force[3];
    int result = simReadForceSensor(sensor, force, nullptr);
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

}  // namespace CS
