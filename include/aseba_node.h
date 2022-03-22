#ifndef ASEBA_NODE_H_INCLUDED
#define ASEBA_NODE_H_INCLUDED

#include <map>
#include <vector>
#include <valarray>
#include <cstdlib>

#include "vm/vm.h"
#include "vm/natives.h"
#include "common/productids.h"
#include "common/consts.h"
#include "common/utils/utils.h"
#include "common/utils/FormatableString.h"
#include "transport/buffer/vm-buffer.h"
#include "stubs.h"


#define VARIABLES_TOTAL_SIZE 1024
#define ID 0
#define SOURCE 1
#define BYTECODE_SIZE 1534
#define STACK_SIZE 32

char *dydata(std::string value);

extern "C" const AsebaNativeFunctionDescription* default_functions_description[];
extern "C" const AsebaLocalEventDescription default_events_description[];
extern "C" const AsebaVMDescription default_variables_description;
extern "C" const AsebaNativeFunctionPointer default_functions[];

class DynamicAsebaNode
{

  std::valarray<unsigned short> bytecode;
  std::valarray<signed short> stack;
public:
  virtual const AsebaNativeFunctionDescription** native_functions_description () const {
    return default_functions_description;
  }
  virtual const AsebaLocalEventDescription* native_events_description () const {
    return default_events_description;
  }
  virtual const AsebaVMDescription* native_variables_description () const {
    return &default_variables_description;
  }
  virtual const AsebaNativeFunctionPointer* native_functions () const {
    return default_functions;
  }

protected:
  int16_t variables[VARIABLES_TOTAL_SIZE];

public:
  bool finalized;
  int script_id;
  std::string name;
  AsebaVMState vm;
  AsebaNativeFunctionDescription** functions_description;
  AsebaLocalEventDescription *events_description;
  AsebaVMDescription *variables_description;

  uint16_t lastMessageSource;
  std::valarray<uint8_t> lastMessageData;

  Aseba::UnifiedTime lastTime;
  // name -> (pointer, size)
  std::map<std::string, std::pair<int16_t *, unsigned int>> named_variable;
  int16_t *next_variable;
  // name -> id
  std::map<std::string, unsigned int> named_event;
  // [<name, argument sizes>]
  std::vector<std::pair<std::string, std::vector<int>>> lua_functions;
  size_t number_of_native_function;

  void init_descriptions()
  {
    // init variable description
    variables_description = (AsebaVMDescription *) malloc(sizeof(AsebaVMDescription) + 1 * sizeof(AsebaVariableDescription));
    // variables_description->name = dydata(name);
    variables_description->name = name.c_str();
    variables_description->variables[0] = AsebaVariableDescription{0, NULL};
    const AsebaVariableDescription *desc = native_variables_description()->variables;
    for (; desc->size; desc++)
    {
      add_variable(std::string(desc->name), desc->size);
    }
    printf("num of variables: %lu\n", named_variable.size());
    // init event description
    events_description = (AsebaLocalEventDescription *) malloc(sizeof(AsebaLocalEventDescription));
    events_description[0] = AsebaLocalEventDescription{NULL, NULL};
    const AsebaLocalEventDescription *event_desc = native_events_description();
    for (; event_desc->name; event_desc++)
    {
      add_event(std::string(event_desc->name), std::string(event_desc->doc));
    }
    printf("num of events: %lu\n", named_event.size());
    // init function description
    const AsebaNativeFunctionDescription **fun_desc = native_functions_description();
    number_of_native_function = 0;
    for (; *fun_desc; fun_desc++, number_of_native_function++);
    fun_desc = native_functions_description();
    functions_description = (AsebaNativeFunctionDescription **) calloc(number_of_native_function + 1, sizeof(AsebaNativeFunctionDescription));
    std::copy((AsebaNativeFunctionDescription **)fun_desc, (AsebaNativeFunctionDescription **)(fun_desc + number_of_native_function + 1), functions_description);
    printf("num of functions: %lu\n", number_of_native_function);
  }

 private:

  void free_descriptions()
  {
    // TODO (J): check and see if I cannot avoid some of these dynamic allocations, especially for strings.
    // e.g. I could use a a pointer to a key stored in my variables (like `named_variable` or `node.name` ...)
    printf("B free_descriptions\n");
    size_t number = named_variable.size();
    for (size_t i = 0; i < number; i++) {
      free((void *)variables_description->variables[i].name);
    }
    // free((void *) variables_description->name);
    free(variables_description);
    number = named_event.size();
    for (size_t i = 0; i < number; i++) {
      free((void *)events_description[i].name);
      free((void *)events_description[i].doc);
    }
    free(events_description);

    number = lua_functions.size();
    for (size_t i = 0; i < number; i++) {
      AsebaNativeFunctionDescription *function = functions_description[number_of_native_function + i];
      free((void *)function->name);
      free((void *)function->doc);
      AsebaNativeFunctionArgumentDescription* arg = function->arguments;
      for (; arg->size; arg++) {
        free((void* )arg->name);
      }
      free(function);
    }
    free(functions_description);
    printf("E free_descriptions\n");
  }

public:
  DynamicAsebaNode(int node_id, std::string _name, int script_id):
  script_id(script_id),
  finalized(false),
  name(_name)
  {
    // setup variables
    vm.nodeId = (int32_t) node_id;
    bytecode.resize(BYTECODE_SIZE);
    vm.bytecode = &bytecode[0];
    vm.bytecodeSize = bytecode.size();
    stack.resize(STACK_SIZE);
    vm.stack = &stack[0];
    vm.stackSize = stack.size();
    vm.variables = reinterpret_cast<int16_t *>(&variables);
    vm.variablesSize = sizeof(variables) / sizeof(int16_t);
    AsebaVMInit(&vm);
    variables[ID] = vm.nodeId;
    next_variable = variables;
    // init_descriptions();
    // printf("name %s %s\n", name.c_str(), variables_description->name);
  }
  ~DynamicAsebaNode()
  {
    free_descriptions();
    printf("Deleted node %s\n", name.c_str());
  }

  virtual void step(float dt)
  {
    AsebaVMRun(&vm, 1000);
  }

  void add_variable(std::string name, unsigned int size)
  {
    printf("add_variable %s of size %d\n", name.c_str(), size);
    if(named_variable.count(name))
    {
      printf("Variable %s cannot be added: already defined\n", name.c_str());
      return;
    }
    size_t number = named_variable.size();
    if(next_variable + size > variables + VARIABLES_TOTAL_SIZE)
    {
      printf("Variable %s cannot be added: not enough free space\n", name.c_str());
      return;
    }
    AsebaVMDescription * tmp = (AsebaVMDescription *) realloc(variables_description, sizeof(AsebaVMDescription) + (number + 2) * sizeof(AsebaVariableDescription));
    if(!tmp)
    {
      printf("Failed to realloc variables_description!\n");
      return;
    }
    variables_description = tmp;
    named_variable[name] = std::make_pair(next_variable, size);
    variables_description->variables[number] = AsebaVariableDescription{(uint16_t) size, dydata(name)};
    variables_description->variables[number+1] = AsebaVariableDescription{0, NULL};
    next_variable += size;
    printf("added variable\n");
  }

  std::vector<int> get_variable(std::string name)
  {
    if(!named_variable.count(name)) return std::vector<int>();
    auto pair = named_variable[name];
    int16_t *address = pair.first;
    size_t size= pair.second;
    std::vector<int> value;
    value.assign(address, address + size);
    return value;
  }

  void set_variable(std::string name, std::vector<int> value)
  {
    if(!named_variable.count(name)) return;
    auto pair = named_variable[name];
    int16_t *address = pair.first;
    size_t size= std::min((unsigned int) value.size(), pair.second);
    std::copy(value.begin(), value.begin() + size, address);
  }

  void add_event(std::string name, std::string description)
  {
    if(named_event.count(name))
    {
      printf("Event %s cannot be added: already defined\n", name.c_str());
      return;
    }
    size_t number = named_event.size();
    events_description = (AsebaLocalEventDescription *) realloc(events_description, (number + 2) * sizeof(AsebaLocalEventDescription));
    if(!events_description)
    {
      printf("Failed to realloc events_description!\n");
      return;
    }
    named_event[name] = number;
    events_description[number] = AsebaLocalEventDescription{dydata(name), dydata(description)};
    events_description[number+1] = AsebaLocalEventDescription{NULL, NULL};
  }

  // ! Execute a local event, killing the execution of the current one if not in step-by-step mode
  void emit(std::string name) {
    if(!named_event.count(name)) return;
    unsigned int number = named_event[name];
    emit(number);
  }

  void emit(uint16_t number) {
  // in step-by-step, only setup an event if none is being executed currently
  if (AsebaMaskIsSet(vm.flags, ASEBA_VM_STEP_BY_STEP_MASK) && AsebaMaskIsSet(vm.flags, ASEBA_VM_EVENT_ACTIVE_MASK))
    return;

  variables[SOURCE] = vm.nodeId;
  AsebaVMSetupEvent(&vm, ASEBA_EVENT_LOCAL_EVENTS_START-number);
  AsebaVMRun(&vm, 1000);
}


  void add_function(std::string name, std::string description, std::vector<argument_t> arguments, std::string callback_name)
  {
    for(auto &f : lua_functions)
    {
      if(f.first == name)
      {
        printf("Function %s cannot be added: already defined\n", name.c_str());
        return;
      }
    }
    size_t number = lua_functions.size() + number_of_native_function;
    std::vector<int> sizes;
    functions_description = (AsebaNativeFunctionDescription **) realloc(functions_description, (number + 2) * sizeof(AsebaNativeFunctionDescription*));
    if(!functions_description)
    {
      printf("Failed to realloc functions_description!\n");
      return;
    }
    AsebaNativeFunctionDescription * desc = (AsebaNativeFunctionDescription *) malloc(sizeof(AsebaNativeFunctionDescription) + (1 + arguments.size()) * sizeof(AsebaNativeFunctionArgumentDescription));
    if(!desc)
    {
        printf("Failed to malloc AsebaNativeFunctionDescription!\n");
        return;
    }
    desc->name = dydata(name);
    desc->doc = dydata(description);
    AsebaNativeFunctionArgumentDescription* arg = desc->arguments;
    for(auto &v : arguments)
    {
      arg->name = dydata(v.name);
      arg->size = v.size;
      arg++;
      sizes.push_back(v.size);
    }
    arg->name = NULL;
    arg->size = 0;
    functions_description[number] = desc;
    functions_description[number+1] = NULL;
    lua_functions.push_back(make_pair(callback_name, sizes));
  }

  void connect()
  {
    finalized = true;
  }

  void do_step(double dt) {};

};


#endif // ASEBA_NODE_H_INCLUDED
