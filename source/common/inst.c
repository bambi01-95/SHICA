#if COMPILER_C
    #include "../compiler/setting.h"
#else
    #include "../executor/setting.h"
#endif

#ifndef INST_C
#define INST_C

enum instrac{
HALT,
i_load,/* value */
l_load,/* value */
f_load,/* value */
d_load,/* value */
c_load,/* value */
s_load,/* value */
k_load,/* value */
//in progress
il_load,/* vnt adress */
ll_load,/* vnt adress */
fl_load,/* vnt adress */
dl_load,/* vnt adress */

i_EQ,
i_NE,
i_LT,
i_LE,
i_GE,
i_GT,
i_AND,
i_OR,
i_ADD,
i_SUB,
i_MUL,
i_DIV,
i_MOD,
i_BAND,
i_BOR,
i_LSH,
i_RSH,
l_EQ,
l_NE,
l_LT,
l_LE,
l_GE,
l_GT,
l_ADD,
l_SUB,
l_MUL,
l_DIV,
l_MOD,
f_EQ,
f_NE,
f_LT,
f_LE,
f_GE,
f_GT,
f_ADD,
f_SUB,
f_MUL,
f_DIV,
d_EQ,
d_NE,
d_LT,
d_LE,
d_GE,
d_GT,
d_ADD,
d_SUB,
d_MUL,
d_DIV,

s_EQ,
s_NE,
s_LT,
s_LE,
s_GE,
s_GT,
s_AND,
s_OR,
s_ADD,
s_SUB,
s_MUL,
s_DIV,
s_MOD,

EOE,   /* end of event function*/
EOC,   /* end of condition */
EOA,   /* end of advice */
COND,  /* event's condtion arugment */
TRANS, /*transition: state change*/
MKCORE,  /*make core: core X:X is number of core*/
MKTHREAD,/* make thread for event */
SETTHREAD,/* set thread for event */
STARTIMP, /* start implement core*/
SET_EVENT,
GLOBAL,
GLOBAL_END,
ENTRY,
/*don't change order start*/
GET,
GET_L,
GET_G,
DEFINE,
DEFINE_L, /* pos */
DEFINE_G, /* pos */
DEFINE_List,    /* vnt index */ //in progress
DEFINE_List_L,  /* vnt index */
DEFINE_List_G,  /* vnt index */
/*don't change order end*/
CALL,
CALL_P,
CALL_A,
SETQ,
RET,
MSUB,
MPOP,
MPICK,
MSET,//global value size settting
JUMP,
JUMPF,
i_PRINT,
l_PRINT,
f_PRINT,
d_PRINT,
c_PRINT,
s_PRINT,
};

#if SBC
//NEED TO SEPARATE
char* INSTNAME[s_PRINT + 1] = {
"HALT",
"i_load",/* value */
"l_load",/* value */
"f_load",/* value */
"d_load",/* value */
"c_load",/* value */
"s_load",/* value */
"k_load",/* value */
//in progress
"il_load",/* vnt adress */
"ll_load",/* vnt adress */
"fl_load",/* vnt adress */
"dl_load",/* vnt adress */

"i_EQ",
"i_NE",
"i_LT",
"i_LE",
"i_GE",
"i_GT",
"i_AND",
"i_OR",
"i_ADD",
"i_SUB",
"i_MUL",
"i_DIV",
"i_MOD",
"i_BAND",
"i_BOR",
"i_LSH",
"i_RSH",
"l_EQ",
"l_NE",
"l_LT",
"l_LE",
"l_GE",
"l_GT",
"l_ADD",
"l_SUB",
"l_MUL",
"l_DIV",
"l_MOD",
"f_EQ",
"f_NE",
"f_LT",
"f_LE",
"f_GE",
"f_GT",
"f_ADD",
"f_SUB",
"f_MUL",
"f_DIV",
"d_EQ",
"d_NE",
"d_LT",
"d_LE",
"d_GE",
"d_GT",
"d_ADD",
"d_SUB",
"d_MUL",
"d_DIV",

"s_EQ",
"s_NE",
"s_LT",
"s_LE",
"s_GE",
"s_GT",
"s_AND",
"s_OR",
"s_ADD",
"s_SUB",
"s_MUL",
"s_DIV",
"s_MOD",

"EOE",   /* end of event function*/
"EOC",   /* end of condition */
"EOA",   /* end of advice */
"COND",
"TRANS", /*transition: state change*/
"MKCORE",  /*make core: core X:X is number of core*/
"MKTHREAD",/* make thread for event */
"SETTHREAD",/* set thread for event */
"STARTIMP",
"SET_EVENT",
"GLOBAL",
"GLOBAL_END",
"ENTRY",
/*don't change order start*/
"GET",
"GET_L",
"GET_G",
"DEFINE",
"DEFINE_L", /* pos */
"DEFINE_G", /* pos */
"DEFINE_List",    /* vnt index */ //in progress
"DEFINE_List_L",  /* vnt index */
"DEFINE_List_G",  /* vnt index */
/*don't change order end*/
"CALL",
"CALL_P",
"CALL_A",
"SETQ",
"RET",
"MSUB",
"MPOP",
"MPICK",
"MSET",//global value size settting
"JUMP",
"JUMPF",
"i_PRINT",
"l_PRINT",
"f_PRINT",
"d_PRINT",
"c_PRINT",
"s_PRINT",
};
#endif

#endif