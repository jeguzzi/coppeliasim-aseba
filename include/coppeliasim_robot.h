#ifndef COPPELIASIM_ROBOT_H
#define COPPELIASIM_ROBOT_H

#include <array>
#include <memory>
#include <fstream>
#include <filesystem>

#include <simPlusPlus/Lib.h>
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
  float angular_speed;
  // sensed
  float angle;
  bool first;
  // integrated (starts at 0 and then continuos)
  float odometry;
  simInt handle;
  float speed() const {
    return radius * angular_speed;
  }
  void update_sensing(float dt);
  void set_target_speed(float speed);
  float get_target_speed() const {
    return nominal_angular_target_speed * radius;
  }
  Wheel(float radius, simInt handle=-1) :
    radius(radius), angular_speed(0.0), first(true), odometry(0.0), handle(handle),
    nominal_angular_target_speed(0.0) {}
 private:
  float nominal_angular_target_speed;
};

struct ProximitySensor {
  float value;
  bool detected;
  simInt handle;
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
      simInt handle_=-1, float min_value=0, float max_value=0, float x0=0, float lambda=0) :
  handle(handle_), detected(false), active(true), only_red(false),
  min_value(min_value), max_value(max_value), x0(x0), lambda(lambda) {}
  void update_sensing(float dt);
};

struct GroundSensor {
  float reflected_light;
  // float ambient_light;
  simInt handle;
  bool active;
  bool only_red;
  bool use_vision;
  // int16_t reflected() const {
  //   return static_cast<int16_t>(reflected_light);
  // }
  // int16_t delta() const {
  //   return static_cast<int16_t>(reflected_light - ambient_light);
  // }
  // static constexpr float max_value = 1032.0;
  static constexpr float min_value = 0.0;
  // static constexpr float x0 = 0.0084;
  static constexpr float max_value = 1291.8;
  static constexpr float x0 = 0.0;
  static constexpr float lambda = 0.01631;
  // static constexpr float lambda = 0.0192;
  void update_sensing(float dt);
  GroundSensor(simInt handle_=-1);
private:
  simInt vision_handle;
};

struct Accelerometer {
  float values[3];
  simInt handle;
  bool active;
  Accelerometer(int handle_=-1);
  void update_sensing(float dt);
private:
  float mass;
  simInt sensor;
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
  simInt handle;

 public:
  Robot(simInt handle);
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

  void enable_ground(bool value, bool red = false, bool vision = false) {
    for (auto & sensor : ground_sensors) {
      sensor.active = value;
      sensor.only_red = red;
      sensor.use_vision = vision;
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
