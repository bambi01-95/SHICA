# SHICA
SHICA event programing

├── README.md
├── lib
│   └── mingc: written by prof Ian Piumarta
│       ├── Makefile
│       ├── bench-fib.txt
│       ├── fatal.c
│       ├── msgc.c: mark and sweep
│       ├── mstest.c
│       ├── rcgc.c: refference counter
│       ├── rctest.c
│       ├── test-cycle.txt
│       ├── test-opt.txt
│       ├── test-sys.txt
│       ├── test.txt
│       └── vmgen.leg
├── src
│   ├── Makefile
│   ├── code
│   ├── code.c
│   ├── code.stt
│   ├── compiler
│   │   └── compiler.c
│   ├── executer
│   │   └── executer.c
│   ├── input.txt
│   ├── inst
│   │   └── inst.c
│   ├── library
│   │   ├── lib_compile
│   │   │   ├── lib_compile.c
│   │   │   └── stdlib-compile.c
│   │   ├── lib_execute
│   │   │   ├── lib_execute.c
│   │   │   └── stdlib-execute.c
│   │   ├── library.h
│   │   └── stdlib.h
│   ├── object
│   │   ├── object.c
│   │   └── object.h
│   ├── parser
│   │   ├── leg: https://www.piumarta.com/software/peg/
│   │   ├── parser.c
│   │   ├── parser.h
│   │   └── parser.leg
│   └── print
│       └── print.c
└── test
    ├── Makefile
    ├── code.stt
    ├── for-test.txt
    ├── if-ineq-test.txt
    ├── if-number-test.txt
    └── while-test.txt


call gc_alloc() 
=> call and imp. gc_collect() 
=> after gc_collect(), threads:OK 
=> continue imp. gc_alloc(), alloc and realloc ...
=> memory clashshshshshshshsh


a
    t
        s
        q
    t
        s
        q
        
realloc? ? ? ?  ? ? ? ? ? ?