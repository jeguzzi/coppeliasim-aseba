#ifndef LOGGING_H
#define LOGGING_H

#pragma warning(disable : 4996)

#include <stdexcept>

#include "simPlusPlus/Lib.h"

template<typename ... Args>
void log(simInt verbosity, const char * format, Args ... args) {
  int size_s = std::snprintf(nullptr, 0, format, args ...) + 1;
  if (size_s <= 0) { throw std::runtime_error( "Error during formatting." ); }
  auto size = static_cast<size_t>(size_s);
  char * buf = new char[ size ];
  std::snprintf(buf, size, format, args ...);
  simAddLog("Aseba", verbosity, buf);
  delete[] buf;
}

#define log_debug(...) log(sim_verbosity_debug, __VA_ARGS__)
#define log_info(...) log(sim_verbosity_infos, __VA_ARGS__)
#define log_warn(...) log(sim_verbosity_warnings, __VA_ARGS__)
#define log_error(...) log(sim_verbosity_errors, __VA_ARGS__)

#endif /* end of include guard: LOGGING_H */
