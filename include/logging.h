#ifndef LOGGING_H
#define LOGGING_H

#ifndef LOG_PRINT

#pragma warning(disable : 4996)

#include <stdexcept>

#include "simPlusPlus/Lib.h"

// template<typename ... Args>
// void log(simInt verbosity, const char * format, Args ... args) {
//   int size_s = std::snprintf(nullptr, 0, format, args ...) + 1;
//   if (size_s <= 0) { throw std::runtime_error( "Error during formatting." ); }
//   auto size = static_cast<size_t>(size_s);
//   char * buf = new char[ size ];
//   std::snprintf(buf, size, format, args ...);
//   simAddLog("Aseba", verbosity, buf);
//   delete[] buf;
// }

template<typename ... Args>
void log(int verbosity, const std::string & format, Args&&... args) {
  simAddLog("Aseba", verbosity, sim::util::sprintf(format, std::forward<Args>(args)...).c_str());
}

#define log_debug(...) log(sim_verbosity_debug, __VA_ARGS__)
#define log_info(...) log(sim_verbosity_infos, __VA_ARGS__)
#define log_warn(...) log(sim_verbosity_warnings, __VA_ARGS__)
#define log_error(...) log(sim_verbosity_errors, __VA_ARGS__)

#else

#define log_debug(...) {printf(__VA_ARGS__); printf("\n");}
#define log_info(...)  {printf(__VA_ARGS__); printf("\n");}
#define log_warn(...)  {printf(__VA_ARGS__); printf("\n");}
#define log_error(...) {printf(__VA_ARGS__); printf("\n");}

#endif

#endif /* end of include guard: LOGGING_H */
