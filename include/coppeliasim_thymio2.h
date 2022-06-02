#ifndef COPPELIASIM_THYMIO2_H
#define COPPELIASIM_THYMIO2_H

#include <array>
#include <memory>
#include <fstream>
#include <filesystem>

#include <simPlusPlus/Lib.h>
#include <opencv2/opencv.hpp>
#include "coppeliasim_robot.h"

namespace CS {

struct SDCard {
  std::filesystem::path path;
  std::fstream stream;
  int file_number;
  bool enabled;

  std::filesystem::path path_for_number(unsigned number) {
    return path / std::filesystem::path(std::to_string(number) + ".DAT");
  }

  void close() {
    if (stream.is_open()) {
      stream.close();
      file_number = -1;
    }
  }

  void set_path(const std::string & value) {
    path = std::filesystem::path(value);
    enabled = std::filesystem::exists(path);
    if (stream.is_open()) {
      stream.close();
      file_number = -1;
    }
  }

  void disable() {
    close();
    enabled = false;
  }

  bool is_open() const {
    return enabled && stream.is_open();
  }

  std::streamsize write(const char * s, std::streamsize count) {
    if (enabled && stream.is_open()) {
      stream.write(s, count);
      return count;
    }
    return 0;
  }

  std::streamsize read(char * s, std::streamsize count) {
    if (enabled && stream.is_open()) {
      stream.read(s, count);
      if (!stream) {
        return stream.gcount();
      }
      return count;
    }
    return 0;
  }

  bool seek(int seek) {
    if (!enabled) return false;

    if (!stream && file_number >= 0) {
      open(file_number);
    }

    if (!stream) return false;

    stream.seekp(seek);
    stream.seekg(seek);

    if (!stream) {
      return false;
    }
    return true;
  }

  bool open(int number) {
    if (!enabled) return false;
    // Adapted from Aseba Playground
    // close current file, ignore errors
    if (stream.is_open()) {
      stream.close();
      file_number = -1;
    }
    // if we have to open another file
    if (number >= 0) {
      const char * file_path = path_for_number(number).c_str();
      stream.open(file_path, std::ios::in | std::ios::out | std::ios::binary);
      if (stream.fail()) {
        // failed... maybe the file does not exist, try with trunc
        stream.open(file_path, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
      }
      if (stream.fail()) {
        // still failed, then it is an error
        return false;
      } else {
        file_number = number;
      }
    }
    return true;
  }

  SDCard() : stream(), path(), enabled(false), file_number(-1) {};


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

class Thymio2 : public Robot {

 class Behavior;

 private:
  std::array<LED, LED::COUNT> leds;
  std::array<Button, Button::COUNT> buttons;
  ProximityComm prox_comm;
  cv::Mat texture;
  simInt texture_id;
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

  SDCard sd_card;

  static constexpr float min_temperature = 0.0;
  static constexpr float max_temperature = 100.0;

 public:
  Thymio2(simInt handle, uint8_t default_behavior_mask = 0x0);
  ~Thymio2();

  virtual void update_sensing(float dt);
  virtual void update_actuation(float dt);

  static constexpr float proximity_min_value = 1000.0;
  static constexpr float proximity_max_value = 4505.0;

  static constexpr float min_battery_voltage = 3.0;
  static constexpr float max_battery_voltage = 4.2;

  void enable_behavior(bool value, uint8_t mask=0xFF);

  float get_battery_voltage() const {
    return battery_voltage;
  }

  void set_battery_voltage(float value) {
    battery_voltage = std::clamp(value, min_battery_voltage, max_battery_voltage);
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
  float get_led_intensity(size_t index) const;
  float get_led_channel(size_t index, size_t channel) const;
  void set_led_channel(size_t index, size_t channel, float value);
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

  bool sd_open(unsigned number) {
    return sd_card.open(number);
  }

  bool sd_seek(int number) {
    return sd_card.seek(number);
  }

  std::streamsize sd_read(char * buffer, std::streamsize count) {
    return sd_card.read(buffer, count);
  }

  std::streamsize sd_write(const char * buffer, std::streamsize count) {
    return sd_card.write(buffer, count);
  }

  void sd_enable(const std::string & value) {
    if (value.empty()) {
      sd_card.disable();
    } else {
      sd_card.set_path(value);
    }
  }

  bool sd_is_enabled() const {
    return sd_card.enabled;
  }

  std::string sd_path() const {
    return sd_card.path;
  }

};

}

#endif /* end of include guard: COPPELIASIM_THYMIO2_H */
