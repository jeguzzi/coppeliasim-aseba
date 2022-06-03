#ifndef COPPELIASIM_EPUCK_H
#define COPPELIASIM_EPUCK_H

#include <array>
#include <memory>
#include <fstream>
#include <filesystem>

#include <simPlusPlus/Lib.h>
#include <opencv2/opencv.hpp>
#include "coppeliasim_robot.h"

namespace CS {

struct Camera {
  simInt handle;
  int image_width;
  int image_height;
  std::vector<uint8_t> image;
  bool active;

  void update_sensing(float dt);

  const uint8_t * get_line(float y) const {
    int i = std::clamp<int>(image_height * y, 0, image_height - 1);
    return image.data() +  image_width * i * 3;
  }

  Camera(simInt handle=-1) :
    handle(handle), image_width(60), image_height(60), image(image_width * image_height * 3),
    active(true) {}
};

struct Gyroscope {
  float values[3];
  simInt handle;
  bool active;
  Gyroscope(int handle=-1) : values(), handle(handle), active(true) {};
  void update_sensing(float dt);
};

class LEDRing {

  struct LED {
    int position;
    bool value;
    cv::Mat on_texture;
    cv::Mat off_texture;
    static constexpr float size_x = 8.0;
    static constexpr float size_y = 14.0;
    static constexpr int y = 258;
    static constexpr float a = 1.5;
    static constexpr int patch_width = 40;
    static constexpr int patch_height = 60;
    void push(simInt texture_id, int texture_size);
    explicit LED(int position, const cv::Mat & texture);
  };

  private:
    // static constexpr std::array<int, 8> positions = {0, 75, 150, 225, 300, 375, 450, 525};
    static constexpr std::array<int, 8> positions = {369, 304, 238, 173, 106, 41, 500, 434};
    static constexpr int texture_size = 1024;
    cv::Mat texture;
    int texture_id;
    simInt shape_handle;
    std::vector<LED> leds;
  public:
    explicit LEDRing(simInt shape_handle=-1);
    void reset(bool);
    void set_value(size_t index, bool value);
    bool get_value(size_t index) const {
      if (index < leds.size()) {
        return leds[index].value;
      }
      return false;
    };
};

class EPuck : public Robot {

 private:

   Camera camera;
   Gyroscope gyroscope;
   std::array<float, 3> mic_intensity;
   LEDRing leds;
   float battery_voltage;
   uint8_t selector;
   uint8_t rc;
   bool front_led;
   bool body_led;
   simInt body_handle;
   simInt ring_handle;
   simInt rest_handle;
   simInt front_led_handle;

 public:
  EPuck(simInt handle);
  ~EPuck();

  virtual void update_sensing(float dt);
  virtual void update_actuation(float dt);

  void reset();
  const uint8_t * get_camera_line(float y) const {
    return camera.get_line(y);
  }

  float get_mic_intensity(size_t i) const {
    if (i < 3) {
      return mic_intensity[i];
    }
    return 0.0;
  }

  void set_mic_intensity(std::vector<float> values) {
    size_t l = std::min(values.size(), mic_intensity.size());
    std::copy_n(values.begin(), l, mic_intensity.begin());
  }

  float get_battery_voltage() const {
    return battery_voltage;
  }

  void set_battery_voltage(float value) {
    battery_voltage = std::clamp<float>(value, 3.0, 4.2);
  }

  uint8_t get_selector() const {
    return selector;
  }

  void set_selector(uint8_t value) {
    selector = value & 0xF;
  }

  uint8_t get_rc() const {
    return rc;
  }

  void set_rc(uint8_t value) {
    rc = value;
  }

  float get_angular_velocity(size_t index) const {
    if(index < 3) return gyroscope.values[index];
    return 0.0;
  }

  void set_ring_led(size_t index, bool value) {
    leds.set_value(index, value);
  }
  void set_body_led(bool value);
  void set_front_led(bool value);
  bool get_ring_led(size_t index) const {
    return leds.get_value(index);
  }
  bool get_body_led() const {
    return body_led;
  }
  bool get_front_led() const {
    return front_led;
  }

  void enable_camera(bool value) {
    camera.active = true;
  }
};

}

#endif /* end of include guard: COPPELIASIM_EPUCK_H */
