#ifndef COPPELIASIM_THYMIO2_H
#define COPPELIASIM_THYMIO2_H

#include <array>
#include <memory>
#include <simPlusPlus/Lib.h>
#include <opencv2/opencv.hpp>

namespace CS {

struct Wheel {
  enum {
    LEFT, RIGHT
  };

  float angular_speed;
  simInt handle;
  static constexpr float radius = 0.027;
  float speed() const {
    return radius * angular_speed;
  }
  void update_sensing(float dt);
  void set_target_speed(float speed);
  float get_target_speed() const {
    return nominal_angular_target_speed * radius;
  }
  Wheel(simInt handle=-1) : angular_speed(0.0), handle(handle), nominal_angular_target_speed(0.0) {}
 private:
  float nominal_angular_target_speed;
};

struct Button {
  enum {
    BACKWARD,
    LEFT,
    CENTER,
    FORWARD,
    RIGHT,
    COUNT
  };
  bool value;
  simInt handle;
  Button(simInt handle_ = -1) : value(false), handle(handle_) {}
};

struct ProximitySensor {
  float value;
  bool detected;
  simInt handle;
  bool active;
  bool only_red;
  static constexpr float max_value = 4505.0;
  static constexpr float min_value = 1000.0;
  static constexpr float x0 = 0.0003;
  static constexpr float lambda = 0.0857;
  int16_t saturated_value() const {
    if (!detected) return 0;
    return static_cast<int16_t>(value);
  }
  ProximitySensor(simInt handle_=-1) : handle(handle_), detected(false), active(true), only_red(false) {}
  void update_sensing(float dt);
};

struct ProxCommMsg {
  std::array<float, 7> intensities;
  std::array<int, 7> payloads;
  int rx;
};

struct ProximityComm {
  bool enabled;
  simInt tx;
  std::vector<ProxCommMsg> rx_buffer;
  std::array<simInt, 7> sensor_handles;
  std::array<simInt, 7> emitter_handles;
  ProximityComm() :
    enabled(false), tx(0), rx_buffer(), sensor_handles(), emitter_handles()  {};
  void update_sensing(const std::array<simInt, 7> & tx_handles, simInt tx);
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

struct LED {
  enum {
    TOP = 0,
    BOTTOM_LEFT, BOTTOM_RIGHT,

    BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT,

    RING_0, RING_1, RING_2, RING_3,
    RING_4, RING_5, RING_6, RING_7,

    IR_FRONT_0, IR_FRONT_1, IR_FRONT_2,
    IR_FRONT_3, IR_FRONT_4, IR_FRONT_5,

    IR_BACK_0, IR_BACK_1,

    LEFT_RED, LEFT_BLUE, RIGHT_BLUE, RIGHT_RED,

    IR_GROUND_0, IR_GROUND_1,

    BATTERY_0, BATTERY_1, BATTERY_2,

    COUNT
  };
  Color color;
};


enum BEHAVIOR {
  LEDS_BATTERY = 1,
  LEDS_BUTTON = 2,
  LEDS_PROX = 4,
  LEDS_ACC = 8,
  LEDS_NTC = 16,
  LEDS_RC5 = 32,
  LEDS_MIC = 64
};

class Thymio2 {

 class Behavior;

 private:
  std::array<Wheel, 2> wheels;
  std::array<ProximitySensor, 7> proximity_sensors;
  std::array<GroundSensor, 2> ground_sensors;
  std::array<LED, LED::COUNT> leds;
  std::array<Button, Button::COUNT> buttons;
  Accelerometer accelerometer;
  ProximityComm prox_comm;
  cv::Mat texture;
  simInt texture_id;
  simInt handle;
  simInt body_handle;

  std::unique_ptr<Behavior> behavior;
  float battery_voltage;
  float temperature;
  float mic_intesity;
  float mic_threshold;
  uint8_t r5_address;
  uint8_t r5_command;
  bool r5;
  uint8_t default_behavior_mask;

  static constexpr float min_temperature = 0.0;
  static constexpr float max_temperature = 100.0;

 public:
  Thymio2(simInt handle, uint8_t default_behavior_mask = 0x0);
  ~Thymio2();
  void set_target_speed(size_t index, float speed);
  float get_target_speed(size_t index);
  virtual void update_sensing(float dt);
  virtual void update_actuation(float dt);
  virtual void do_step(float dt);

  static constexpr float min_battery_voltage = 3.0;
  static constexpr float max_battery_voltage = 4.2;

  void enable_behavior(bool value, uint8_t mask=0xFF);

  float get_battery_voltage() const {
    return battery_voltage;
  }

  void set_battery_voltage(float value) {
    battery_voltage = std::clamp(value, min_battery_voltage, max_battery_voltage);
  }

  const float * get_acceleration_values() const {
    return accelerometer.values;
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

  bool get_button(size_t index) {
    if (index < Button::COUNT) {
      return buttons[index].value;
    }
    return false;
  }

  void set_button(size_t index, bool value) {
    if (index < Button::COUNT) {
      buttons[index].value = value;
    }
  }

  std::vector<simInt> button_handles() const {
    std::vector<simInt> handles;
    for (auto & button : buttons) {
      handles.push_back(button.handle);
    }
    return handles;
  }

  const std::vector<ProxCommMsg> & prox_comm_rx() const {
    return prox_comm.rx_buffer;
  }

  int prox_comm_tx() const {
    return prox_comm.tx;
  }

  void set_prox_comm_tx(simInt tx) {
    // printf("robot set_prox_comm_tx %d\n", tx);
    prox_comm.tx = tx;
  }

  void enable_prox_comm(bool value) {
    prox_comm.enabled = value;
  }

  const std::array<simInt, 7> & prox_comm_emitter_handles() const {
    return prox_comm.emitter_handles;
  }

  bool prox_comm_enabled() const {
    return prox_comm.enabled;
  }

  void update_prox_comm(const std::array<simInt, 7> & emitter_handles, simInt tx) {
    prox_comm.update_sensing(emitter_handles, tx);
  }

  void reset_prox_comm_rx() {
    prox_comm.rx_buffer.clear();
  }

  void set_led_color(size_t index, bool force, float r = 0, float g = 0, float b = 0);
  void set_led_intensity(size_t index, float intensity);
  void reset_texture(bool reload = false);
  void reset();
  bool had_collision() const {return false;}

  void set_temperature(float value) {
    temperature = std::clamp(value, min_temperature, max_temperature);
  }

  float get_temperature() const {
    return temperature;
  }

  void set_mic_intensity(float intensity) {
    mic_intesity = std::clamp(intensity, 0.0f, 1.0f);
  }

  float get_mic_intensity() const {
    return mic_intesity;
  }

  void set_mic_threshold(float threshold) {
    mic_threshold = std::clamp(threshold, 0.0f, 1.0f);
  }

  float get_mic_threshold() const {
    return mic_threshold;
  }

  void receive_rc_message(uint8_t address, uint8_t command) {
    // R5 protocol (https://en.wikipedia.org/wiki/RC-5): 5-bit address, 6-bit command
    r5_address = address & 0x1F;
    r5_command = command & 0x3F;
    r5 = true;
  }

  bool received_rc_message(int16_t & address, int16_t & command) const {
    if (r5) {
      address = r5_address;
      command = r5_command;
    }
    return r5;
  }

  bool received_rc_message() const {
    return r5;
  }

};

}

#endif /* end of include guard: COPPELIASIM_THYMIO2_H */
