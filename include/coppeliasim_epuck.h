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

class EPuck : public Robot {

 private:

   Camera camera;
   Gyroscope gyroscope;
   std::array<float, 3> mic_intensity;
   float battery_voltage;
   uint8_t selector;
   uint8_t rc;

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

};

}

#endif /* end of include guard: COPPELIASIM_EPUCK_H */
