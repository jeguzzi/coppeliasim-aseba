#ifndef COPPELIASIM_THYMIO2_H
#define COPPELIASIM_THYMIO2_H

#include <array>
#include <simPlusPlus/Lib.h>
#include <opencv2/opencv.hpp>

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

  LED_COUNT
};

enum {
  LEFT_WHEEL, RIGHT_WHEEL
};


class CoppeliaSimThymio2 {
  struct Wheel {
    float angular_speed;
    simInt handle;
    static constexpr float radius = 0.027;
    float speed() const {
      return radius * angular_speed;
    }
    void update_sensing(float dt);
    void set_target_speed(float speed);
    Wheel(simInt handle=-1) : angular_speed(0.0), handle(handle), nominal_angular_target_speed(0.0) {}
   private:
    float nominal_angular_target_speed;
  };

  struct ProximitySensor {
    float value;
    bool detected;
    simInt handle;
    int16_t saturated_value() const {
      if (!detected) return 0;
      return static_cast<int16_t>(value);
    }
    ProximitySensor(simInt handle_=-1) : handle(handle_), detected(false) {}
    void update_sensing(float dt);
  };

  struct GroundSensor {
    float reflected_light;
    // float ambient_light;
    simInt handle;
    // int16_t reflected() const {
    //   return static_cast<int16_t>(reflected_light);
    // }
    // int16_t delta() const {
    //   return static_cast<int16_t>(reflected_light - ambient_light);
    // }
    void update_sensing(float dt);
    GroundSensor(simInt handle_=-1);
  private:
    simInt vision_handle;
  };

  struct Accelerometer {
    float values[3];
    simInt handle;
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
    Color color;
  };

 private:
  std::array<Wheel, 2> wheels;
  std::array<ProximitySensor, 7> proximity_sensors;
  std::array<GroundSensor, 2> ground_sensors;
  std::array<LED, LED_COUNT> leds;
  Accelerometer accelerometer;
  cv::Mat texture;
  simInt texture_id;

 public:
  CoppeliaSimThymio2(simInt handle);
  ~CoppeliaSimThymio2();
  void set_target_speed(size_t index, float speed);
  virtual void update_sensing(float dt);
  virtual void update_actuation(float dt);
  virtual void do_step(float dt);

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

  void set_led_color(size_t index, bool force, float r = 0, float g = 0, float b = 0);
  void set_led_intensity(size_t index, float intensity);
  void reset_texture();
  void reset();
  bool had_collision() const {return false;}
};

#endif /* end of include guard: COPPELIASIM_THYMIO2_H */
