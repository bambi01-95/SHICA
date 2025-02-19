# SHICA

## Event Oriented Programming Language for Malti Agent System

This is a programming language for real-world multi-agent systems inspired by LSL. It provides event management for each state.

## Features

- State machine like sysntax (LSL)
- Event ...
- Multi action, one event

## Sample Code of SHICA

This is a code that prints "Hello World!":

```shica
state default{
    entry(){
        print("Hello World!")
    }
}
```

SHICA handles events within a state. The following is an example of LED management code that handles two events, timerSec() and gpioPinRead():

```shica
state default{ //default state is initially implemented
    entry(){
        gpioSet(13,0) //set pin 13 for OUTPUT
        init gpioPinRead(17,1) set gpio 17 pin read event 
        init timerSec(10) //set timer event every 10 sec
        state off //state transistion to off state
    }
}

state off{
    entry(){
        print("LED OFF")
        gpioWrite(13,0) //LED off
    }
    gpioPinRead(){
        state on
    }
}

state on{
    entry(){
        print("LED ON")
        timerSec.reset(0) //reset timer 0
        gpioWrite(13,1) //LED on
    }
    timerSec(int s){
        state off
    }
    gpioPinRead(){
        state off
    }
}
```

This code manages LED on and off using two states: on and off. It will transition to the opposite state when Button (pin 17) is cliked. And also, on state will transistion to
off state after 10 seconds.

## Documentation

Currently, no documentation is provided.Please reffer ./test/.

## Development Status

### Under Development – Not Ready for Use

This project is still under development and not ready for public use.

If you are interested in using it, please contact us first.

## Version?

v1.0.0 First Release: multi action and eval args cond

v1.0.1 Optimaze eval event args

v1.0.2 RPI GPIO lib and Wifi UDP lib

## Setup

### To set up the development environment, please follow the steps below

1. Clone this repository:

    ```sh
    d-.-b b
    ```

2. compile SHICA compiler:

    ```sh
    cd ../source
    make com
    ```

3. compile SHICA virtual machin:

    ```sh
    cd ../source
    make vms
    ```

## Implementation

### To implement the shica code, please follow the steps below

1. Write a code by reading and refergin sample code at ./test.

2. Compile by shica compiler:

    ```sh
    ./shica yor_code.txt
    ```

3. Run shica code by shica vm:

    ```sh
    ./vm yor_code.stt
    ```

## Affiliation

This project is developed by Deer d-.-b at Kyoto University of Advanced Science,  
Graduate School of Engineering, Programming System Laboratory.

## Other

### Secound Life (Linden Scripting Language)

[Second Life home page](https://secondlife.com/)

[LSL Portal](https://wiki.secondlife.com/wiki/LSL_Portal)

### Parser (PEG/LEG)

[peg/leg — recursive-descent parser generators for C](https://www.piumarta.com/software/peg/)

### KUAS

[KUAS home page](https://www.kuas.ac.jp/)
