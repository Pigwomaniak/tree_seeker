# TreeSeeker
Code for autonomus drone to find sick trees 

## Table of contents
* [General info](#general-info)
* [Screenshots](#screenshots)
* [Technologies](#technologies)
* [Setup](#setup)
* [Features](#features)
* [Status](#status)
* [Inspiration](#inspiration)
* [Contact](#contact)

## General info
Add more general information about project. What the purpose of the project is? Motivation?

## Screenshots
[Example screenshot](./img/screenshot.png)

## Technologies
* Ros noetic
* Gazebo 11
* Ardupilot

## Setup
To start using clone repository to catkin workspace where you keep source code and ru `catkin build`

This repository is connected with [drone_sim](https://github.com/Pigwomaniak/drone_sim) repository where whole simiulation gazebo files are stored.

## Code Examples
To run example program:
`$ rosrun tree_seeker drone_ridder`

## Features
Now the main node is [drone_riddeer](./src/drone_ridder.cpp) this node will provide 5 topisc to publish in for drone control.

* "drone_ridder/set_mode" message `std_msgs::String` where y can rquest of mode hange

###To-do

* "drone_ridder/set_position_offset" message `geometry_msgs::Point` where values x, y, z are vector of movment in ENU.
* "drone_ridder/set_local_position" message `geometry_msgs::Point` where values x, y, z are destination coordinates in local ENU.
* "drone_ridder/heading" message `std_msgs::Float64` where value represent YAW angle. 0 is when x is facing Est and risig value i counter clockwise.
* "drone_ridder/set_global_position" message `sensor_msgs::NavSatFix` set LLA destination possion

This is erly stage of development and none of features are well tested but few functions seems to work fine
* addin waypoints in adrucopter local coordinate system
* setting heading
* arming
* flight mode hange

## Status
Project is: _in progress_

## Inspiration
Project inspired by [Inteligent-Quads](https://github.com/Intelligent-Quads)

## Contact
Created by [Pigwomaniak](https://github.com/Pigwomaniak) - feel free to contact me!
