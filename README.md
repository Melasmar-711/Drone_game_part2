# Drone Navigation System
## Overview
This project implements a real-time drone navigation system using shared memory and process synchronization in C. The system consists of multiple components that work together to control a drone's movement in a 2D environment while avoiding obstacles.

## Features
- Portable pub-sub structure that allows game component to be replaced by other providers

## Dependencies
- gcc compiler
- ncurses library
- Standard C libraries (stdio.h, stdlib.h, etc.)
- FasrDDS v3.1.0

## Installation
1. `git clone https://github.com/Melasmar-711/Drone_game_part2.git `
2. Ensure all dependencies are installed .
3. set the values you wish to cJson file.
4. Compile the run the project at the same time using the run.sh:
    ```bash
    ./run.sh publisher    # Starts the publisher processes
    ./run.sh subscriber   # Starts the subscriber processes
    ./run.sh both 
    ```


## Components
- ### **1stmodule** in subscriber mode
- **BlackBoardServer**: Manages the blackboard server for inter-process communication.
- **DroneDynamicsManager**: Handles the dynamics and physics of the drones.
- **GameWindow**: Provides the game window interface using the ncurses library.
- **KeyboardManager**: Manages keyboard inputs.
- **Obstacle_Generator**: sends  obstacles in to the server 
- **Targets_Generator**: sends  targets in to the server 
- **WatchDog**: Monitors the system to ensure all components are running correctly.
-


- ### **2nd module**  in publisher mode
- **Targets_publisher**: publishes the generated Targets by Targets_generator  
- **Targets_Generator**: Generates targets for the drones and send them to the publisher to publish them.
- **Obstacles_publisher**: publishes the generated obstacles by Targets_generator 
- **Obstacle_Generator**: Generates obstacles in the simulation and send them to the publisher to publish them.


## Test Runs
Here are some videos demonstrating the system in action:
- [Basic Movement Demo](#)
- [Full System Test](#)

## Project Structure

example of project structure
```
Drone_game_part2/ 

|── CMakeLists.txt
├── BlackBoardServer/
│   ├── server.h
│   ├── server.c
│   ├── BlackBoardServer.c
|   ├──CMakeLists.txt
│   
|
|
|

```

## Communication Diagram

Below is a text-based diagram illustrating the communication between different nodes in the drone navigation system:

```
+--------------------------------------------------------+
|                    PublisherModule                     |
|                                                        |
|   +-------------------+       +-------------------+    |
|   |                   |       |                   |    |
|   |Obstacle_Generator |       |  Targets_Generator|    |
|   |                   |       |                   |    |
|   +---------+---------+       +---------+---------+    |
|           |                           |                |
|           |                           |                |
|           v                           v                |
|   +---------+---------+       +---------+---------+    |
|   |                   |       |                   |    |
|   |Obstacles_publisher|       |  Targets_publisher|    |
|   |                   |       |                   |    |
|   +---------+---------+       +---------+---------+    |
|                                                        |
|                                                        |
+--------------------------------------------------------+
                        |
    Targets_Topic       |       Obstacles_Topic           #that's why it's portable since publisher publishing the same topics shall work if placed here
                        |
                        v
+--------------------------------------------------------+
|                    Subscriber MOdule                   |
|                                                        |
|   +-------------------+     +-------------------+      |      
|   |                   |     |                   |      |    +----------------------+
|   |Obstacle_subscriber|     |Targets_subscriber |      |    |                      |
|   |                   |     |                   |      |    |WatchDog Monitoring   |
|   +---------+---------+     +---------+---------+      |    |                      |
|           |                           |                |    +----------------------+
|           |                           |                |
|           v                           v                |
|   +---------+---------+       +---------+---------+    |
|   |                   |       |                   |    |
|   |Obstacle_Generator |       |Targets_Generator  |    |
|   |                   |       |                   |    |
|   +---------+---------+       +---------+---------+    |
|           |                           |                |
|           v                           v                |
|        =---------------------------------------+       |
|        |                                       |       -------+---------------------|
|        |                                       |              |                     |
|        |                                       |------------> |    GameWindow       |
|        |           BlackBoardServer            |              |                     |
|        |                                       |       -------+---------------------+
|        +------_--------------------------------+       |
|            | / \                    |                  |
|            v  |                     |                  |
|   +-------------------+     +-------------------+      |
|   |                   |     |                   |      |
|   | DroneDynamicsManager |  | Keyboard          |      |
|   |                   |     |                   |      |
|   +---------+---------+     +---------+---------+      |
|                                                        |
+--------------------------------------------------------+

```

This diagram shows the flow of data from the generators to the publishers, then to the BlackBoardServer, and finally to the various managers and interfaces.



