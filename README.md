# SHICA

## Event Oriented Programming Language for Malti Agent System


    v1.0.0 First Release: multi action and eval args cond

    v1.0.1 Optimaze eval event args

    v1.0.2 RPI GPIO lib and Wifi UDP lib

    .

    ├── README.md  

    ├── source  

    │   ├── code.stt  

    │   ├── common  

    │   │   ├── inst.c  

    │   │   ├── liblist  

    │   │   │   ├── library.h  

    │   │   │   └── stdlib.h  

    │   │   └── memory.c  

    │   ├── compiler  

    │   │   ├── generate.c  

    │   │   ├── lib  

    │   │   │   └── cLangInterface  

    │   │   │       ├── cParser.leg  

    │   │   │       ├── input.txt  

    │   │   │       ├── leg  

    │   │   │       ├── makefile  

    │   │   │       ├── memo.txt  

    │   │   │       ├── output.c  

    │   │   │       ├── test  

    │   │   │       └── test.c  

    │   │   ├── library  

    │   │   │   ├── lib.c  

    │   │   │   └── stdlib-compile.c  

    │   │   ├── object.c  

    │   │   ├── optimaize.c  

    │   │   ├── parser  

    │   │   │   ├── MEMO.txt  

    │   │   │   ├── leg  

    │   │   │   ├── parser.c  

    │   │   │   ├── parser.h  

    │   │   │   └── parser.leg  

    │   │   └── tool.c  

    │   ├── compiler.c  

    │   ├── executor  

    │   │   ├── lib  

    │   │   │   ├── fatal.c  

    │   │   │   └── msgc.c  

    │   │   ├── library  

    │   │   │   ├── lib.c  

    │   │   │   └── stdlib-execute.c  

    │   │   ├── object.c  

    │   │   ├── run.c  

    │   │   ├── setting.h  

    │   │   └── tool.c  

    │   ├── executor.c  

    │   ├── expanded.c  

    │   ├── input.txt  

    │   ├── makefile  

    │   ├── shica  

    │   └── vm  

    └── test  

        ├── Makefile  

        ├── code.stt  

        ├── for-test.txt  

        ├── if-ineq-test.txt  

        ├── if-number-test.txt  

        └── while-test.txt  



[leg](https://www.piumarta.com/software/peg/)