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

class EPuck : public Robot {

 private:

 public:
  EPuck(simInt handle);
  ~EPuck();

  virtual void update_sensing(float dt);
  virtual void update_actuation(float dt);

  void reset();

};

}

#endif /* end of include guard: COPPELIASIM_EPUCK_H */
