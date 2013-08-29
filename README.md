ConVEX
======

The ConVEX firmware library is an add on package for the ChibiOS/RT
operating system that supports the VEX cortex microcontroller.
The library adds the necessary functionality to allow C applications to be
created and executed on the cortex that can access its available hardware.

ConVEX is not trying to be competitive with ROBOTC or EasyC, it is only 
appropriate for advanced users who have some familiarity with RTOS concepts
and are comfortable with programming micro-controllers at a bare bones level.

ConVEX uses many features of the ChibiOS/RT library and the hardware abstraction 
layer it provides for the STM32 processor.  Modification of the low level drivers
could cause damage to the cortex hardware, however, under most circumstances user
code can be developed that is as robust as the existing environments for the cortex.
 
Follow the steps outlined in the [Getting Started](http://jpearman.github.io/convex/doxygen/html/getstarted.html)
section of the [documentation](http://jpearman.github.io/convex/doxygen/html/index.html) to install ConVEX and the necessary tools.  A few demo 
programs are included that can be used as a starting point for
user projects.

### Features
* Preemptive multithreading
* 128 priority levels
* Round-robin scheduling for threads at the same priority level
* Software timers
* Counting semaphores
* Mutexes with support for the priority inheritance algorithm
* Condition variables
* Synchronous and asynchronous Messages
* Event flags and handlers
* Queues
* Synchronous and asynchronous I/O with timeout capability
* Thread-safe memory heap and memory pool allocators.

### Cortex specific features
* Digital IO
* Analog Inputs, potentiometer, accelerometer etc.
* Ultrasonic sensor support
* Quadrature encoder support
* Motor control
* Integrated motor encoder support
* Support for two VEX LCD displays
* Competition control
* Simple audio tones and chip tone playback support
* Command line interface (shell)

### Optional features
* SmartMotor library
* Apollo - real time display of motor and sensor data
* VEX Gyro
* PID library
* ROBOTC style tasks and Sensor support to allow easy code porting