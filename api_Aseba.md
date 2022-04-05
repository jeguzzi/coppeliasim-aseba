## Structures
| struct |
|----|
| [argument_t](#argument_t) |
| [node_t](#node_t) |


## Functions
| function |
|----|
| [simAseba.create_node](#create_node) |
| [simAseba.destroy_node](#destroy_node) |
| [simAseba.set_uuid](#set_uuid) |
| [simAseba.set_friendly_name](#set_friendly_name) |
| [simAseba.add_variable](#add_variable) |
| [simAseba.set_variable](#set_variable) |
| [simAseba.get_variable](#get_variable) |
| [simAseba.add_event](#add_event) |
| [simAseba.emit_event](#emit_event) |
| [simAseba.add_function](#add_function) |
| [simAseba.list_nodes](#list_nodes) |
| [simAseba.destroy_network](#destroy_network) |
| [simAseba.load_script](#load_script) |
| [simAseba.set_script](#set_script) |




#### argument_t
Description of an argument of an Aseba function used in 
```C++
argument_t = {int size = 1, string name }
```
*fields*

  - **size** The argument size

  - **name** The argument name



#### node_t
Aseba node
```C++
node_t = {int port , string name , int id }
```
*fields*

  - **port** The port of the Aseba network the node belongs to.

  - **name** The node name

  - **id** The node id






#### create_node


Create an Aseba node and connect it to an Aseba network. Until the first simulation step is completed, the node can be edited, adding variables, events and functions. After the first pass, its Aseba description will be freezed.
```C++
int id=simAseba.create_node(int handle,int port=33333,int id=-1,string name="node",string friendly_name="",int[] uuid={})
```
*parameters*

  - **handle** Handle of the CoppeliaSim object

  - **port** The Aseba port number

  - **id** The Aseba node ID. Two nodes in the same Aseba network cannot have the same id. Set to -1 to choose the lowest available id.

  - **name** The Aseba node name, which identifies the type of devices. Two nodes in the same Aseba network can have the same name.

  - **friendly_name** A friendlier name used in Thymio Suite to label nodes. Must not be unique. If left empty, it is set to "{name}".

  - **uuid** An 16-byte long uuid to uniquely identify nodes in Thymio Suite: each node should have a different uuid. If left empty, the uuid is set to "coppeliasim {id}".

*return*

  - **id** The ID assigned to the Aseba node




#### destroy_node


Remove an Aseba node from the Aseba network. All Aseba nodes will be eventually destroyed at the end of a simulation run, even if not explicitly requested using this callback.
```C++
simAseba.destroy_node(int id)
```
*parameters*

  - **id** The Aseba node ID.






#### set_uuid


Set the indentifier of an Aseba node in Thymio Suite from Mobsya. Has no effect when using older clients from the Aseba community.
```C++
simAseba.set_uuid(int id,int[] uuid)
```
*parameters*

  - **id** The Aseba node ID

  - **uuid** An array of 16 bytes






#### set_friendly_name


Set the friendly name for an Aseba node in Thymio Suite from Mobsya. Has no effect when using older clients from the Aseba community.
```C++
simAseba.set_friendly_name(int id,string name)
```
*parameters*

  - **id** The Aseba node ID

  - **name** An string shorter than 56 characters.






#### add_variable


Add an Aseba variable to a node. This is only possible after node creation, before the first simulation step. All Aseba variables are named array of a fixed number of 16-bit integers.
```C++
simAseba.add_variable(int id,string name,int size=1)
```
*parameters*

  - **id** The Aseba node ID

  - **name** The variable name

  - **size** The variable size (should be larger than 0)






#### set_variable


Set an Aseba variable. The value array should have same size of the variable and be limited to the 16-bit integers range, i.e., [-32768, 32767]. Higher bits are ignored.
```C++
simAseba.set_variable(int id,string name,int[] value)
```
*parameters*

  - **id** The Aseba node ID

  - **name** The variable name

  - **value** The variable value






#### get_variable


Get an Aseba variable
```C++
int[] value=simAseba.get_variable(int id,string name)
```
*parameters*

  - **id** The Aseba node ID

  - **name** The varible name

*return*

  - **value** The variable value




#### add_event


Add an Aseba event to a node. This is only possible after node creation, before the first simulation step.
```C++
simAseba.add_event(int id,string name,string description)
```
*parameters*

  - **id** The Aseba node ID

  - **name** The event name

  - **description** The event description that will be publicized on the network






#### emit_event


Emit an Aseba event.
```C++
simAseba.emit_event(int id,string name)
```
*parameters*

  - **id** The Aseba node ID

  - **name** The event name






#### add_function


Add an Aseba function to a node. This is only possible after node creation, before the first simulation step. When the Aseba function get called, it triggers a callback in this lua script with the same arguments.
```C++
simAseba.add_function(int id,string name,string description,argument_t[] arguments,string callback)
```
*parameters*

  - **id** The Aseba node ID

  - **name** The function name

  - **description** The function description that will be publicized on the network

  - **arguments** The function arguments, which are also publicized on the network

  - **callback** The name of the lua function that implements the callback in the current script






#### list_nodes


Get a list of simulated Aseba nodes.
```C++
node_t[] nodes=simAseba.list_nodes(int port=-1)
```
*parameters*

  - **port** If different from -1, limit to nodes attached to that port

*return*

  - **nodes** The list of nodes




#### destroy_network


Destroy all nodes of an Aseba Network and disconnect it.
```C++
simAseba.destroy_network(int port=-1)
```
*parameters*

  - **port** The Aseba network port. Use -1 to destroy all networks.






#### load_script


Load an Aseba script into a node from a file.
```C++
bool result=simAseba.load_script(int id,string path)
```
*parameters*

  - **id** The Aseba node ID

  - **path** The path to the script file.

*return*

  - **result** Whether script loading was successfull.




#### set_script


Load an Aseba script into a node from text code. The code cannot define or use user events or constants. If you need them, please use `load_script` instead.
```C++
bool result=simAseba.set_script(int id,string code)
```
*parameters*

  - **id** The Aseba node ID

  - **code** The text code with the Aseba script.

*return*

  - **result** Whether script loading was successfull.


