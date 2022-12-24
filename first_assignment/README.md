# ARP-Hoist-Assignment

## Overview
The following project concerns the implementation of a program that simulates the behavior of a Hoist, a 2 d.o.f system that is controlled by the user using two different consoles. 

## How to Install the program
It is possibile to install the simulator using:

1. In the shell, writing:
```console
	git clone https://github.com/jek97/ARP.git
```
2. Dowloading the zip file directly from Github

Further packages are not required.  

## Compiling and running the code
The Command and Inspection processes depend on the ncurses library, which needs to be linked during the compilation step. Furthermore, the Inspection process also uses the mathematical library for some additional computation. Therefore the steps to compile are the following:
1. for the **Master process**:
	```console
	gcc src/master.c -o bin/master
	```
2. for the **Command process**:
	```console
	gcc src/command_console.c -lncurses -o bin/command
	```
3. for the **Motor 1**:
	```console
	gcc src/motor1.c -lncurses -lm -o bin/motor1
	```	
4. for the **Motor 2**:
	```console
	gcc src/motor2.c -lncurses -lm -o bin/motor2
	```	
5. for the **Error**:
	```console
	gcc src/inspection_console.c -lncurses -lm -o bin/error
	```
6. for the **Inspection process**:
	```console
	gcc src/inspection_console.c -lncurses -lm -o bin/inspection
	```	
After compiling, **assuming you have Konsole installed in your system** as per the professor's indications, you can **simply run the Master executable**, which will be responsible of spawning the two GUIs:
```console
./bin/master
```

## Base Project Structure
Base project structure for the first *Advanced and Robot Programming* (ARP) assignment.
The project provides the basic functionalities for the **Command** and **Inspection processes**, both of which are implemented through the *ncurses library* as simple GUIs. In particular, the repository is organized as follows:
- The `src` folder contains the source code for the Command, Inspection and Master processes.
- The `include` folder contains all the data structures and methods used within the ncurses framework to build the two GUIs. Unless you want to expand the graphical capabilities of the UIs (which requires understanding how ncurses works), you can ignore the content of this folder, as it already provides you with all the necessary functionalities.
- The `bin` folder is where the executable files are expected to be after compilation

## ncurses installation
To install the ncurses library, simply open a terminal and type the following command:
```console
sudo apt-get install libncurses-dev
```

## How the simulator works

The master process controls all the communcations between processes, every process communicates with another using named pipes and signals.
The command console is in charge of recieving the inputs from the user in order to make the hoist move along the directions x and y.
Motor1 and Motor2 simulates the motors whereas the error process adds a random error on the positions to make the problem more real life-like.
In the inspection console the user can either stop the movement of the Hoist or can reset its position.
Lastly, the watchdog checks all the proccesses periodically and sends a reset whether all the processes do nothing for sixty seconds.

## Troubleshooting

Should you experience some weird behavior after launching the application (buttons not spawning inside the GUI or graphical assets misaligned) simply try to resize the terminal window, it should solve the bug.
