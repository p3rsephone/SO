# Commands
This project is designed to support the commands:

| Commands     | Brief summary                                                             |
| ------------ |:------------------------------------------------------------------------------------------:|
| [`const`](COMMANDS.md#const)      | Appends the argument to the input                                                          | 
| [`filter`](COMMANDS.md#filter) | Filters the input with a condition given as the argument                                   | 
| [`window`](COMMANDS.md#window)     | Repeats the input,  with an added column with the result of an operation given as argument | 
| [`spawn`](COMMANDS.md#spawn)      | Executes a command                                                                         | 
| [`node`](COMMANDS.md#node)        | Creates a node                                                                             | 
| [`connect`](COMMANDS.md#connect)     | Connects 2 nodes together                                                                  | 
| [`disconnect`](COMMANDS.md#disconnect)  | Disconnects 2 nodes that were previously connected                                         | 
| [`inject`](COMMANDS.md#inject)      | Injects the result of a command to a node                                                  | 
| [`remove`](COMMANDS.md#remove) | Removes a node from the network                                                            |

## Const
This command reproduces the lines, adding a new column with the same value.

### Usage
`const <value>`

### Example
```
const 10
```
has the following result:

| Input | Output |
| ----- |:------:|
| `a:3:x:4` | `a:3:x:4:10` |
| `b:1:y:10` | `b:1:y:10:10` |
| `a:2:w:2` | `a:2:w:2:10` |
| `d:5:z:34` | `d:5:z:34:10` |

## Filter
This command reproduces the lines that satisfy a condition indicated in its arguments. It supports the operators: >, <, =, <=, >= and !=.

### Usage
`filter <column> <operator> <column>`

### Example
```
filter 2 < 4
```
has the following result:

| Input | Output |
| ----- | ------ |
| `a:3:x:4` | `a:3:x:4` |
| `b:1:y:10` | `b:1:y:10` |
| `a:2:w:2` | `d:5:z:34` |
| `d:5:z:34` |

## Window
This command reproduces all of the lines, adding a new column with the result of an operation that is calculted with the values of a column, amongst several lines. It supports the operators : avg, max, min and sum.

### Usage
`window <column> <operator> <number of previous lines>`

### Example
```
window 4 avg 2
```
has the following result:

| Input | Output | The math |
| ----- | ------ |:--------:|
| `a:3:x:4` | `a:3:x:4:0` | Since there are no 2 previous lines the avg is 0 |
| `b:1:y:10` | `b:1:y:10:4` | Since there is only 1 previous line with the value 4 the avg is 4 |
| `a:2:w:2` | `a:2:w:2:7` | (4+10)/2 = 7 |
| `d:5:z:34` | `d:5:z:34:6`| (10+2)/2 = 6 |

## Spawn
This command reproduces the lines,  executing the given command to each line, and also adding a new line with the *exit status*. A certain column of the input can also be refered in the command with the `$` sign.

### Usage
`spawn <cmd> <args...>`

### Example
```
spawn mailx -s $3 x@y.com
```
has the following result:

| Input | Output |
| ----- | ------ |
| `a:2:w:2` | `a:2:w:2:0` |
| `d:5:z:34` | `d:5:z:34:0`|

and also sends messages with the subject w and the subject z to x@y.com.

## Others
Other commands like `cut`, `grep`, `tee`, `echo`, `ls` and some more are also supported.

# Controller
These are the specific commands used to create/remove/connect/disconnect different nodes. They are the commands that the controller can accept.

## Node
This command redefines an old node or adds a new one to the network. It has an id for later mentioning, and it executes a given command.

### Usage
`node <id> <cmd> <args...>`

### Example
```
node 1 window 2 avg 10
```
specifies that the node 1 executes the command `window` with the arguments `2 avg 10`.

## Connect
This command defines connections between nodes. It means that the output of the node `id` becomes the input of the `ids` list of nodes.

### Usage
`connect <id> <ids...>`

### Example
```
connect 1 2
connect 2 3 5
```
specifies that the output of node 1 is sent to node 2, and the output of node 2 is sent as the input to nodes 3 and 5.

## Disconnect
This command undoes the connection between node `id1` and node `id2`.

### Usage
`disconnect <id1> <id2>`

### Example
```
disconnect 2 3
```
specifies that nodes 2 and 3 are no more connected.

## Inject
This command injects the output of `cmd` with the argument list `args` , to the input of the node `id`. It is normally used when the network is already established, and it opens a new terminal where the user can enter as many lines of input as he whishes.

### Usage
`inject <id> <cmd> <args...>`

### Example
```
inject 1 const 50
```
specifies that the output of `const 50` goes, as the input, to node 1.

## Remove
This command removes the node `id`. It is also guaranteed that all of the nodes that used to connect to `id` are now connected to the node `id` was connected to.

### Usage
`remove <id>`

### Example
```
remove 2
```
specifies that, following the previous [example](COMMANDS.md#example-5),  node 2 does not exist anymore, and node 1 is now connected to 3 and 5.
