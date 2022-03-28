w## Structures
| struct |
|----|
| [argument_t](#argument_t) |
| [node_t](#node_t) |


## Functions
| function |
|----|
| [simAseba.create_node](#create_node) |
| [simAseba.destroy_node](#destroy_node) |
| [simAseba.add_variable](#add_variable) |
| [simAseba.set_variable](#set_variable) |
| [simAseba.get_variable](#get_variable) |
| [simAseba.add_event](#add_event) |
| [simAseba.emit_event](#emit_event) |
| [simAseba.add_function](#add_function) |
| [simAseba.list_nodes](#list_nodes) |
| [simAseba.destroy_network](#destroy_network) |




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
int id=simAseba.create_node(int handle,int port=33333,int id=-1,string name="node")
```
*parameters*

  - **handle** Handle of the CoppeliaSim object

  - **port** The Aseba port number

  - **id** The Aseba node ID. Two nodes in the same Aseba network cannot have the same id. Set to -1 to choose the lowest available id.

  - **name** The Aseba node name. This identify the type of devices. Two nodes in the same Aseba network can have the same name.

*return*

  - **id** ID assigned to the Aseba node




#### destroy_node


Remove an Aseba node from the Aseba network. All Aseba nodes will be eventually destroyed at the end of a simulation run, even if not explicitly requested using this callback.
```C++
simAseba.destroy_node(int id)
```
*parameters*

  - **id** The Aseba node ID.

*return*




#### add_variable


Add an Aseba variable to a node. This is only possible after node creation, before the first simulation step. All Aseba variables are named array of a fixed number of 16-bit integers.
```C++
simAseba.add_variable(int id,string name,int size=1)
```
*parameters*

  - **id** The Aseba node ID

  - **name** The variable name

  - **size** The variable size (should be larger than 0)

*return*




#### set_variable


Set an Aseba variable. The value array should have same size of the variable and be limited to the 16-bit integers range, i.e., [-32768, 32767]. Higher bits are ignored.
```C++
simAseba.set_variable(int id,string name,int[] value)
```
*parameters*

  - **id** The node ID

  - **name** The variable name

  - **value** The variable value

*return*




#### get_variable


Get an Aseba variable
```C++
int[] value=simAseba.get_variable(int id,string name)
```
*parameters*

  - **id** The node ID

  - **name** The varible name

*return*

  - **value** The variable value




#### add_event


Add an Aseba event to a node. This is only possible after node creation, before the first simulation step.
```C++
simAseba.add_event(int id,string name,string description)
```
*parameters*

  - **id** The node ID

  - **name** The event name

  - **description** The event description that will be publicized on the network

*return*




#### emit_event


Emit an Aseba event.
```C++
simAseba.emit_event(int id,string name)
```
*parameters*

  - **id** The node ID

  - **name** The event name

*return*




#### add_function


Add an Aseba function to a node. This is only possible after node creation, before the first simulation step. When the Aseba function get called, it triggers a callback in this lua script with the same arguments.
```C++
simAseba.add_function(int id,string name,string description,argument_t[] arguments,string callback)
```
*parameters*

  - **id** The node ID

  - **name** The function name

  - **description** The function description that will be publicized on the network

  - **arguments** The function arguments, which are also publicized on the network

  - **callback** The name of the lua function that implements the callback in the current script

*return*




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

*return*
