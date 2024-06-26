#ifndef COPPELIASIM_ROBOT_H
#define COPPELIASIM_ROBOT_H

#include <array>
#include <memory>
#include <fstream>
#include <filesystem>

#include <simPlusPlus/Lib.h>

#if SIM_PROGRAM_VERSION_NB >= 40500
typedef double simFloat;
#else
typedef float simFloat;
#endif

#include <opencv2/opencv.hpp>

namespace CS {

// struct Sensor {
//   virtual void update_sensing(float dt) = 0;
// };

struct Wheel {
  enum {
    LEFT, RIGHT
  };
  const float radius;
  simFloat angular_speed;
  // sensed
  simFloat angle;
  bool first;
  // integrated (starts at 0 and then continuos)
  float odometry;
  int handle;
  float speed() const {
    return radius * angular_speed;
  }
  void update_sensing(float dt);
  void set_target_speed(float speed);
  float get_target_speed() const {
    return nominal_angular_target_speed * radius;
  }
  Wheel(float radius, int handle=-1) :
    radius(radius), angular_speed(0.0), first(true), odometry(0.0), handle(handle),
    nominal_angular_target_speed(0.0) {}
 private:
  float nominal_angular_target_speed;
};

struct ProximitySensor {
  float value;
  bool detected;
  int handle;
  bool active;
  bool only_red;
  const float min_value;
  const float max_value;
  const float x0;
  const float lambda;
  int16_t saturated_value() const {
    if (!detected) return 0;
    return static_cast<int16_t>(value);
  }
  ProximitySensor(
      int handle_=-1, float min_value=0, float max_value=0, float x0=0, float lambda=0) :
  handle(handle_), detected(false), active(true), only_red(false),
  min_value(min_value), max_value(max_value), x0(x0), lambda(lambda) {}
  void update_sensing(float dt);
};

struct GroundSensor {
  float reflected_light;
  // float ambient_light;
  int handle;
  bool active;
  bool only_red;
  bool use_vision;
  static constexpr float min_value = 0.0;
  #if 0
  static constexpr float max_value = 1291.8;
  static constexpr float x0 = 0.0;
  static constexpr float lambda = 0.01631;
  #endif
  constexpr static float default_max_value = 1185.5;
  constexpr static float default_x0 = 0.00864;
  float max_value;
  float x0;
  void update_sensing(float dt);
  GroundSensor(int handle_=-1);
private:
  int vision_handle;
};

struct Accelerometer {
  float values[3];
  int handle;
  bool active;
  Accelerometer(int handle_=-1);
  void update_sensing(float dt);
private:
  simFloat mass;
  int sensor;
};

struct Color {
  float r, g, b, a;

  explicit Color(float r = 0, float g = 0, float b = 0) :
    r(r), g(g), b(b), a(0) {}

  operator bool() const {
    return a && r && g && b;
  }

  bool set_a(float a_) {
    if (a != a_) {
      a = a_;
      return true;
    }
    return false;
  }

  bool set_rgb(float r_, float g_, float b_) {
    float a_ = std::max({r_, g_, b_});
    if (a_ > 0) {
      r_ /= a_;
      g_ /= a_;
      b_ /= a_;
    }
    if((a == a_ && r == r_ && g == g_ && b == b_)) return false;
    a = a_;
    r = r_;
    g = g_;
    b = b_;
    return true;
  }
};

class Robot {

 protected:
  std::vector<Wheel> wheels;
  std::vector<ProximitySensor> proximity_sensors;
  std::vector<GroundSensor> ground_sensors;
  Accelerometer accelerometer;
  int handle;

 public:
  Robot(int handle);
  ~Robot();
  void set_target_speed(size_t index, float speed);
  float get_target_speed(size_t index);
  virtual void update_sensing(float dt);
  virtual void update_actuation(float dt);
  virtual void do_step(float dt);

  const float * get_acceleration_values() const {
    return accelerometer.values;
  }
  float get_odometry(size_t index) const {
    if(index < wheels.size()) return wheels[index].odometry;
    return 0.0;
  }
  float get_speed(size_t index) const {
    if(index < wheels.size()) return wheels[index].speed();
    return 0.0;
  }
  int16_t get_proximity_value(size_t index) const {
    if(index < proximity_sensors.size()) return proximity_sensors[index].saturated_value();
    return 0;
  }
  int16_t get_ground_reflected(size_t index) const {
    if(index < ground_sensors.size()) return ground_sensors[index].reflected_light;
    return 0;
  }
  int16_t get_ground_delta(size_t index) {
    if(index < ground_sensors.size()) return ground_sensors[index].reflected_light;
    return 0;
  }
  float get_acceleration(size_t index) const {
    if(index < 3) return accelerometer.values[index];
    return 0.0;
  }
  void enable_accelerometer(bool value) {
    accelerometer.active = value;
  }

  void enable_ground(bool value, bool red = false, bool vision = false, 
                     float max_value=GroundSensor::default_max_value, 
                     float x0=GroundSensor::default_x0) {
    for (auto & sensor : ground_sensors) {
      sensor.active = value;
      sensor.only_red = red;
      sensor.use_vision = vision;
      sensor.max_value = max_value;
      sensor.x0 = x0;
    }
  }

  void enable_proximity(bool value, bool red = false) {
    for (auto & sensor : proximity_sensors) {
      sensor.active = value;
      sensor.only_red = red;
    }
  }

  void reset();

};

}

#endif /* end of include guard: COPPELIASIM_ROBOT_H */
