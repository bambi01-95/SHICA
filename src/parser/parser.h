#ifndef OBJECT_H
#define OBJECT_H
#include "../object/object.c"
#endif

#ifndef PARSER
#define PARSER

extern oop result;
extern int newline;
int yyparse();

#endif