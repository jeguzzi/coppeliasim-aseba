#ifndef ASEBAROS_INCLUDE_ASEBAROS_ASEBASCRIPT_H
#define ASEBAROS_INCLUDE_ASEBAROS_ASEBASCRIPT_H

#include <map>
#include <memory>

#if defined(_WIN32)
#undef ERROR_STACK_OVERFLOW
#endif

#include "compiler/compiler.h"


// UTF8 to wstring
inline std::wstring widen(const char *src) {
  const size_t destSize(mbstowcs(0, src, 0) + 1);
  std::vector<wchar_t> buffer(destSize, 0);
  mbstowcs(&buffer[0], src, destSize);
  return std::wstring(buffer.begin(), buffer.end() - 1);
}

inline std::wstring widen(const std::string &src) { return widen(src.c_str()); }

// wstring to UTF8
inline std::string narrow(const wchar_t *src) {
  const size_t destSize(wcstombs(0, src, 0) + 1);
  std::vector<char> buffer(destSize, 0);
  wcstombs(&buffer[0], src, destSize);
  return std::string(buffer.begin(), buffer.end() - 1);
}

inline std::string narrow(const std::wstring &src) { return narrow(src.c_str()); }

class AsebaScript {

  typedef std::map<std::string, std::map<unsigned, std::string>> CodeMap;

public:
  // static std::shared_ptr<AsebaScript> from_string(const std::string &value);
  static std::shared_ptr<AsebaScript> from_file(const std::string &path);
  static std::shared_ptr<AsebaScript> from_code_string(
      const std::string &value, std::string & name, unsigned id);

  bool compile(unsigned node_id, const Aseba::TargetDescription *description,
               Aseba::VariablesMap &variable_map,
               Aseba::BytecodeVector &bytecode);
  AsebaScript() : common_definitions(), code(){};
  // user events and constants
  Aseba::CommonDefinitions common_definitions;
  // type -> (id -> aesl text)
  CodeMap code;
  std::string source;
};

#endif // ASEBAROS_INCLUDE_ASEBAROS_ASEBASCRIPT_H
