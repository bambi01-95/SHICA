# SHICA

## Event Oriented Programming Language for Malti Agent System

This is a programming language for real-world multi-agent systems inspired by LSL.  
It provides event management for each state.

## Under Development – Not Ready for Use

This project is still under development and not ready for public use.

If you are interested in using it, please contact us first.

## Version?

    v1.0.0 First Release: multi action and eval args cond

    v1.0.1 Optimaze eval event args

    v1.0.2 RPI GPIO lib and Wifi UDP lib

## Setup / How to use

### To set up the development environment, please follow the steps below

1. Clone this repository:

    ```sh
        d-.-b b
    ```

2. compile SHICA compiler:
    cd ../source
    make com

3. compile SHICA virtual machin:
    cd ../source
    make vms

### To implement the shica code, please follow the steps below

1. Write a code by reading and refergin sample code at ./test.

2. Compile by shica compiler:
    ./shica yor_code.txt

3. Run shica code by shica vm:
    ./vm yor_code.stt

## Affiliation

This project is developed by Deer d-.-b at Kyoto University of Advanced Science,  
Graduate School of Engineering, Programming System Laboratory.

## Other

### Linden Scripting Language

[Second Life home page](https://secondlife.com/)

[LSL Portal](https://wiki.secondlife.com/wiki/LSL_Portal)

### Parser (PEG/LEG)

[peg/leg — recursive-descent parser generators for C](https://www.piumarta.com/software/peg/)
