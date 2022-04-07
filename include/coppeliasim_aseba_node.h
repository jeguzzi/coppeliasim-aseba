#ifndef COPPELIASIM_ASEBA_NODE_H_INCLUDED
#define COPPELIASIM_ASEBA_NODE_H_INCLUDED

#include <stack>
#include <vector>
#include <cstdint>

#include "logging.h"
#include "aseba_node.h"


struct LuaFunction {
  unsigned script_id;
  const std::string name;
  const std::vector<int> argument_sizes;
  LuaFunction(
      unsigned script_id, const std::string & name, const std::vector<int> & argument_sizes) :
      script_id(script_id), name(name), argument_sizes(argument_sizes) {}
};

class CoppeliaSimAsebaNode : public DynamicAsebaNode {
 private:
  std::vector<LuaFunction> lua_functions;
  unsigned script_id;

 protected:
  void free_descriptions()
  {
    for (size_t i = 0; i < lua_functions.size(); i++) {
      AsebaNativeFunctionDescription *function = functions_description[number_of_native_function + i];
      free((void *)function->name);
      free((void *)function->doc);
      AsebaNativeFunctionArgumentDescription* arg = function->arguments;
      for (; arg->size; arg++) {
        free((void* )arg->name);
      }
      free(function);
    }
    DynamicAsebaNode::free_descriptions();
  }

public:

  using DynamicAsebaNode::DynamicAsebaNode;

  void set_script_id(unsigned id) {
    script_id = id;
  }

  void add_function(const std::string & name, const std::string & description,
                    const std::vector<std::tuple<int, std::string>> & arguments,
                    const std::string & callback_name)
  {
    log_debug("Try to add function %s", name.c_str());
    for(const auto & f : lua_functions)
    {
      if(f.name == name)
      {
        log_warn("Function %s cannot be added: already defined", name.c_str());
        return;
      }
    }
    size_t number = lua_functions.size() + number_of_native_function;
    std::vector<int> sizes;
    functions_description = (AsebaNativeFunctionDescription **) realloc(functions_description, (number + 2) * sizeof(AsebaNativeFunctionDescription*));
    if(!functions_description)
    {
      log_warn("Failed to realloc functions_description!");
      return;
    }
    AsebaNativeFunctionDescription * desc = (AsebaNativeFunctionDescription *) malloc(sizeof(AsebaNativeFunctionDescription) + (1 + arguments.size()) * sizeof(AsebaNativeFunctionArgumentDescription));
    if(!desc)
    {
        log_warn("Failed to malloc AsebaNativeFunctionDescription!");
        return;
    }
    desc->name = dydata(name);
    desc->doc = dydata(description);
    AsebaNativeFunctionArgumentDescription* arg = desc->arguments;
    for(auto &v : arguments)
    {
      arg->name = dydata(std::get<1>(v));
      arg->size = std::get<0>(v);
      sizes.push_back(arg->size);
      arg++;
    }
    arg->name = NULL;
    arg->size = 0;
    functions_description[number] = desc;
    functions_description[number+1] = NULL;
    lua_functions.emplace_back(script_id, callback_name, sizes);
    log_debug("Added function");
  }

  void call_function(AsebaVMState *vm, unsigned id) override {
    if (id < lua_functions.size()) {
      int stack_id = simCreateStack();
      // How many arguments?
      auto & function = lua_functions[id];
      int number_of_arguments = function.argument_sizes.size();
      std::stack<uint16_t> addresses;
      // TODO(Jerome): take into account dynamic lengths (i.e. size = -1, -2 , ...)
      for (auto size : function.argument_sizes) {
        uint16_t address = AsebaNativePopArg(vm);
        addresses.push(address);
        // TODO(Jerome): variable length arrays are not part of any C++ standart!
        // Replace witha a vector -> vector.data ~(or array) no, it only accept
        // const expr as sizes~ int32_t values[size];
        std::vector<int32_t> values(size);
        for (size_t i = 0; i < size; i++) {
          values[i] = (int32_t)vm->variables[address + i];
        }
        simPushInt32TableOntoStack(stack_id, values.data(), size);
        // printf("IN [%d] = [%d, ...]\n", address, values[0]);
      }
      // printf("IN stack size %d\n", simGetStackSize(stack_id));
      log_debug("Will call lua function %s for script %d with %d arguments",
                function.name.c_str(), function.script_id, number_of_arguments);
      int res = simCallScriptFunctionEx(function.script_id, function.name.c_str(), stack_id);
      log_debug("Has called lua function -> %d", res);
      int number_of_results = simGetStackSize(stack_id);
      // printf("OUT stack size %d\n", number_of_results);
      int num = std::min(number_of_arguments, number_of_results);
      for (int i = 0; i < num; i++) {
        int size = function.argument_sizes[number_of_arguments - i - 1];
        uint16_t address = addresses.top();
        // int32_t values[size];
        // TODO(Jerome): check that it works!!!
        std::vector<int32_t> values(size);
        simGetStackInt32Table(stack_id, values.data(), size);
        // printf("OUT [%d] = [%d, ...]\n", address, values[0]);
        for (size_t i = 0; i < size; i++) {
          // TODO(Jerome): check that the casting is correct
          vm->variables[address + i] = (uint16_t)values[i];
        }
        simPopStackItem(stack_id, 1);
        addresses.pop();
      }
      simReleaseStack(stack_id);
    }
  }
};


#endif // COPPELIASIM_ASEBA_NODE_H_INCLUDED
