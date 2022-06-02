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

  void update_sensing(float dt);

  const uint8_t * get_line(float y) const {
    int i = std::clamp<int>(image_height * y, 0, image_height - 1);
    return image.data() +  image_width * i * 3;
  }

  Camera(simInt handle=-1) :
    handle(handle), image_width(60), image_height(60), image(image_width * image_height * 3) {}
};

class EPuck : public Robot {

 private:

   Camera camera;

 public:
  EPuck(simInt handle);
  ~EPuck();

  virtual void update_sensing(float dt);
  virtual void update_actuation(float dt);

  void reset();
  const uint8_t * get_camera_line(float y) const {
    return camera.get_line(y);
  }

};

}

#endif /* end of include guard: COPPELIASIM_EPUCK_H */
