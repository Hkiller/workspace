#include <assert.h>
#include "cpe/pal/pal_string.h"
#include "cpe/pal/pal_stdlib.h"
#include "cpe/utils/string_utils.h"
#include "cpe/utils/buffer.h"
#include "cpe/utils/stream_buffer.h"
#include "xcalc_context_i.h"
#include "xcalc_computer_i.h"

#define  __def_state(num) ((uint32_t)num)

#define state_begin  __def_state(0)
#define state_eq     __def_state(1)
#define state_bt     __def_state(2)
#define state_lt     __def_state(3)
#define state_and    __def_state(4)
#define state_or     __def_state(5)
#define state_num1   __def_state(6)
#define state_num2   __def_state(7)
#define state_letter __def_state(8)
#define state_str1   __def_state(9)
#define state_str2   __def_state(10)
#define state_not1   __def_state(11)
#define state_sub    __def_state(12)
#define state_var    __def_state(13)
#define state_end    0x00008000u
#define state_back   0x00004000u

#define __state_num 14

#define __make_node(state, token) ( state | (token << 16) )
#define __make_node_back(state, token) ( state | (token << 16) | state_back )

#define __token(node) ((uint32_t)( (node & 0xffff0000) >> 16 ))
#define __state(node) ((uint32_t)( node & 0x000000ff))
#define __is_end(node) (node & state_end)
#define __need_back(node) (node & state_back)

#define node_none       __make_node     (state_end, XTOKEN_ERROR)
#define node_add        __make_node     (state_end, XTOKEN_ADD)
#define node_sub        __make_node     (state_sub, 0)
#define node_sub2       __make_node_back(state_end, XTOKEN_SUB)
#define node_mul        __make_node     (state_end, XTOKEN_MUL)
#define node_div        __make_node     (state_end, XTOKEN_DIV)
#define node_eq1        __make_node     (state_eq, 0)
#define node_eq         __make_node     (state_end, XTOKEN_EQU)
#define node_bg1        __make_node     (state_bt, 0)
#define node_bg         __make_node_back(state_end, XTOKEN_BG)
#define node_bg_nb      __make_node     (state_end, XTOKEN_BG)
#define node_be         __make_node     (state_end, XTOKEN_BE)
#define node_lt1        __make_node     (state_lt, 0)
#define node_lt         __make_node_back(state_end, XTOKEN_LT)
#define node_lt_nb      __make_node     (state_end, XTOKEN_LT)
#define node_le         __make_node     (state_end, XTOKEN_LE)
#define node_and1       __make_node     (state_and, 0)
#define node_and        __make_node     (state_end, XTOKEN_AND)
#define node_or1        __make_node     (state_or, 0)
#define node_or         __make_node     (state_end, XTOKEN_OR)
#define node_not1       __make_node     (state_not1, 0)
#define node_not        __make_node_back(state_end, XTOKEN_NOT)
#define node_not_nb     __make_node     (state_end, XTOKEN_NOT)
#define node_ne         __make_node     (state_end, XTOKEN_NE)
#define node_lbr        __make_node     (state_end, XTOKEN_LEFT_BRACKET)
#define node_rbr        __make_node     (state_end, XTOKEN_RIGHT_BRACKET)
#define node_colon      __make_node     (state_end, XTOKEN_COLON)
#define node_qes        __make_node     (state_end, XTOKEN_QES)
#define node_comma      __make_node     (state_end, XTOKEN_COMMA)
#define node_func       __make_node     (state_end, XTOKEN_FUNC)
#define node_end        __make_node_back(state_end, XTOKEN_END)
#define node_num1       __make_node     (state_num1, 0)
#define node_int        __make_node_back(state_end, XTOKEN_NUM_INT)
#define node_int_nb     __make_node     (state_end, XTOKEN_NUM_INT)
#define node_num2       __make_node     (state_num2, 0)
#define node_double     __make_node_back(state_end, XTOKEN_NUM_FLOAT)
#define node_double_nb  __make_node     (state_end, XTOKEN_NUM_FLOAT)
#define node_let1       __make_node     (state_letter, 0)
#define node_str1       __make_node     (state_str1, 0)
#define node_str2       __make_node     (state_str2, 0)
#define node_str        __make_node     (state_end, XTOKEN_STRING)
#define node_str_nb     __make_node_back(state_end, XTOKEN_STRING)
#define node_var1       __make_node     (state_var, 0)
#define node_var        __make_node_back(state_end, XTOKEN_VAL)
#define node_begin      __make_node     (state_begin, 0)

static const uint32_t node_table[__state_num][256] = {
    /*state_begin*/
    {
        /*  0*/ node_end      , node_none     , node_none     , node_none     , node_none     ,
        /*  5*/ node_none     , node_none     , node_none     , node_none     , node_begin    ,
        /* 10*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 15*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 20*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 25*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 30*/ node_none     , node_none     , node_begin    , node_not1     , node_str1     ,
        /* 35*/ node_none     , node_none     , node_none     , node_and1     , node_str2     ,
        /* 40*/ node_lbr      , node_rbr      , node_mul      , node_add      , node_comma    ,
        /* 45*/ node_sub      , node_none     , node_div      , node_num1     , node_num1     ,
        /* 50*/ node_num1     , node_num1     , node_num1     , node_num1     , node_num1     ,
        /* 55*/ node_num1     , node_num1     , node_num1     , node_colon    , node_none     ,
        /* 60*/ node_lt1      , node_eq1      , node_bg1      , node_qes      , node_var1     ,
        /* 65*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 70*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 75*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 80*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 85*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 90*/ node_let1     , node_none     , node_none     , node_none     , node_none     ,
        /* 95*/ node_none     , node_none     , node_let1     , node_let1     , node_let1     ,
        /*100*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*105*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*110*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*115*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*120*/ node_let1     , node_let1     , node_let1     , node_none     , node_or1      ,
        /*125*/ node_none     , node_none     , node_none     , node_let1     , node_let1     ,
        /*130*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*135*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*140*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*145*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*150*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*155*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*160*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*165*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*170*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*175*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*180*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*185*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*190*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*195*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*200*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*205*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*210*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*215*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*220*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*225*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*230*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*235*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*240*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*245*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*250*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*255*/ node_let1
    }
    ,
    /*state_eq*/
    {
        /*  0*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*  5*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 10*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 15*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 20*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 25*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 30*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 35*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 40*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 45*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 50*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 55*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 60*/ node_none     , node_eq       , node_none     , node_none     , node_none     ,
        /* 65*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 70*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 75*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 80*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 85*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 90*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 95*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*100*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*105*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*110*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*115*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*120*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*125*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*130*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*135*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*140*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*145*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*150*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*155*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*160*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*165*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*170*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*175*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*180*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*185*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*190*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*195*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*200*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*205*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*210*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*215*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*220*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*225*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*230*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*235*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*240*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*245*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*250*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*255*/ node_none
    }
    ,
    /*state_bt*/
    {
        /*  0*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*  5*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg_nb    ,
        /* 10*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 15*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 20*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 25*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 30*/ node_bg       , node_bg       , node_bg_nb    , node_bg       , node_bg       ,
        /* 35*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 40*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 45*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 50*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 55*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 60*/ node_bg       , node_be       , node_bg       , node_bg       , node_bg       ,
        /* 65*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 70*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 75*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 80*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 85*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 90*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /* 95*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*100*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*105*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*110*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*115*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*120*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*125*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*130*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*135*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*140*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*145*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*150*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*155*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*160*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*165*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*170*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*175*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*180*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*185*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*190*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*195*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*200*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*205*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*210*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*215*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*220*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*225*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*230*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*235*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*240*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*245*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*250*/ node_bg       , node_bg       , node_bg       , node_bg       , node_bg       ,
        /*255*/ node_bg
    }
    ,
    /*state_lt*/
    {
        /*  0*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*  5*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt_nb    ,
        /* 10*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 15*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 20*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 25*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 30*/ node_lt       , node_lt       , node_lt_nb    , node_lt       , node_lt       ,
        /* 35*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 40*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 45*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 50*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 55*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 60*/ node_lt       , node_le       , node_lt       , node_lt       , node_lt       ,
        /* 65*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 70*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 75*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 80*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 85*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 90*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /* 95*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*100*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*105*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*110*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*115*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*120*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*125*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*130*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*135*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*140*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*145*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*150*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*155*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*160*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*165*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*170*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*175*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*180*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*185*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*190*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*195*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*200*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*205*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*210*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*215*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*220*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*225*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*230*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*235*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*240*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*245*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*250*/ node_lt       , node_lt       , node_lt       , node_lt       , node_lt       ,
        /*255*/ node_lt
    }
    ,
    /*state_and*/
    {
        /*  0*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*  5*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 10*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 15*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 20*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 25*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 30*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 35*/ node_none     , node_none     , node_none     , node_and      , node_none     ,
        /* 40*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 45*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 50*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 55*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 60*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 65*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 70*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 75*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 80*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 85*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 90*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 95*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*100*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*105*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*110*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*115*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*120*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*125*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*130*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*135*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*140*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*145*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*150*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*155*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*160*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*165*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*170*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*175*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*180*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*185*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*190*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*195*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*200*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*205*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*210*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*215*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*220*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*225*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*230*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*235*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*240*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*245*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*250*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*255*/ node_none
    }
    ,
    /*state_or*/
    {
        /*  0*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*  5*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 10*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 15*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 20*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 25*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 30*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 35*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 40*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 45*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 50*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 55*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 60*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 65*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 70*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 75*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 80*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 85*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 90*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /* 95*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*100*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*105*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*110*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*115*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*120*/ node_none     , node_none     , node_none     , node_none     , node_or       ,
        /*125*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*130*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*135*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*140*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*145*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*150*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*155*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*160*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*165*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*170*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*175*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*180*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*185*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*190*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*195*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*200*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*205*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*210*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*215*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*220*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*225*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*230*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*235*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*240*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*245*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*250*/ node_none     , node_none     , node_none     , node_none     , node_none     ,
        /*255*/ node_none
    }
    ,
    /*state_num1*/
    {
        /*  0*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /*  5*/ node_int      , node_int      , node_int      , node_int      , node_int_nb   ,
        /* 10*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /* 15*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /* 20*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /* 25*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /* 30*/ node_int      , node_int      , node_int_nb   , node_int      , node_int      ,
        /* 35*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /* 40*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /* 45*/ node_int      , node_num2     , node_int      , node_num1     , node_num1     ,
        /* 50*/ node_num1     , node_num1     , node_num1     , node_num1     , node_num1     ,
        /* 55*/ node_num1     , node_num1     , node_num1     , node_int      , node_int      ,
        /* 60*/ node_int      , node_int      , node_int      , node_int      , node_int      ,
        /* 65*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 70*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 75*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 80*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 85*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 90*/ node_let1     , node_int      , node_int      , node_int      , node_int      ,
        /* 95*/ node_int      , node_int      , node_let1     , node_let1     , node_let1     ,
        /*100*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*105*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*110*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*115*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*120*/ node_let1     , node_let1     , node_let1     , node_int      , node_int      ,
        /*125*/ node_int      , node_int      , node_int      , node_int      , node_let1     ,
        /*130*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*135*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*140*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*145*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*150*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*155*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*160*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*165*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*170*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*175*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*180*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*185*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*190*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*195*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*200*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*205*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*210*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*215*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*220*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*225*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*230*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*235*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*240*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*245*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*250*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*255*/ node_let1
    }
    ,
    /*state_num2*/
    {
        /*  0*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /*  5*/ node_double   , node_double   , node_double   , node_double   , node_double_nb,
        /* 10*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /* 15*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /* 20*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /* 25*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /* 30*/ node_double   , node_double   , node_double_nb, node_double   , node_double   ,
        /* 35*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /* 40*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /* 45*/ node_double   , node_double   , node_double   , node_num2     , node_num2     ,
        /* 50*/ node_num2     , node_num2     , node_num2     , node_num2     , node_num2     ,
        /* 55*/ node_num2     , node_num2     , node_num2     , node_double   , node_double   ,
        /* 60*/ node_double   , node_double   , node_double   , node_double   , node_double   ,
        /* 65*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 70*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 75*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 80*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 85*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 90*/ node_let1     , node_double   , node_double   , node_double   , node_double   ,
        /* 95*/ node_double   , node_double   , node_let1     , node_let1     , node_let1     ,
        /*100*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*105*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*110*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*115*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*120*/ node_let1     , node_let1     , node_let1     , node_double   , node_double   ,
        /*125*/ node_double   , node_double   , node_double   , node_double   , node_let1     ,
        /*130*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*135*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*140*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*145*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*150*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*155*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*160*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*165*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*170*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*175*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*180*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*185*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*190*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*195*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*200*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*205*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*210*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*215*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*220*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*225*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*230*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*235*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*240*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*245*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*250*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*255*/ node_let1
    }
    ,
    /*state_letter*/
    {
        /*  0*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /*  5*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 10*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 15*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 20*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 25*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 30*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 35*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 40*/ node_func     , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 45*/ node_let1      , node_str_nb      , node_str_nb      , node_let1     , node_let1     ,
        /* 50*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 55*/ node_let1     , node_let1     , node_let1     , node_str_nb      , node_str_nb      ,
        /* 60*/ node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      , node_str_nb      ,
        /* 65*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 70*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 75*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 80*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 85*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 90*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /* 95*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*100*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*105*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*110*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*115*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*120*/ node_let1     , node_let1     , node_let1     , node_str_nb   , node_str_nb   ,
        /*125*/ node_str_nb   , node_str_nb   , node_str_nb   , node_let1     , node_let1     ,
        /*130*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*135*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*140*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*145*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*150*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*155*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*160*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*165*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*170*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*175*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*180*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*185*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*190*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*195*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*200*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*205*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*210*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*215*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*220*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*225*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*230*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*235*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*240*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*245*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*250*/ node_let1     , node_let1     , node_let1     , node_let1     , node_let1     ,
        /*255*/ node_let1
    }
    ,
    /*state_str1*/
    {
        /*  0*/ node_none     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*  5*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 10*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 15*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 20*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 25*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 30*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str      ,
        /* 35*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 40*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 45*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 50*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 55*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 60*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 65*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 70*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 75*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 80*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 85*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 90*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /* 95*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*100*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*105*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*110*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*115*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*120*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*125*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*130*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*135*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*140*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*145*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*150*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*155*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*160*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*165*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*170*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*175*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*180*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*185*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*190*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*195*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*200*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*205*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*210*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*215*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*220*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*225*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*230*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*235*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*240*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*245*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*250*/ node_str1     , node_str1     , node_str1     , node_str1     , node_str1     ,
        /*255*/ node_str1
    }
    ,
    /*state_str2*/
    {
        /*  0*/ node_none     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*  5*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 10*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 15*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 20*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 25*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 30*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 35*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str      ,
        /* 40*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 45*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 50*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 55*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 60*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 65*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 70*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 75*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 80*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 85*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 90*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /* 95*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*100*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*105*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*110*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*115*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*120*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*125*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*130*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*135*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*140*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*145*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*150*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*155*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*160*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*165*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*170*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*175*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*180*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*185*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*190*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*195*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*200*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*205*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*210*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*215*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*220*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*225*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*230*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*235*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*240*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*245*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*250*/ node_str2     , node_str2     , node_str2     , node_str2     , node_str2     ,
        /*255*/ node_str2
    }
    ,
    /*state_not1*/
    {
        /*  0*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*  5*/ node_not      , node_not      , node_not      , node_not      , node_not_nb   ,
        /* 10*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 15*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 20*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 25*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 30*/ node_not      , node_not      , node_not_nb   , node_not      , node_not      ,
        /* 35*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 40*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 45*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 50*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 55*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 60*/ node_not      , node_ne       , node_not      , node_not      , node_not      ,
        /* 65*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 70*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 75*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 80*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 85*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 90*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /* 95*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*100*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*105*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*110*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*115*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*120*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*125*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*130*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*135*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*140*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*145*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*150*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*155*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*160*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*165*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*170*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*175*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*180*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*185*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*190*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*195*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*200*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*205*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*210*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*215*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*220*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*225*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*230*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*235*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*240*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*245*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*250*/ node_not      , node_not      , node_not      , node_not      , node_not      ,
        /*255*/ node_not
    }
    ,
    /*state_sub*/
    {
        /*  0*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*  5*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 10*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 15*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 20*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 25*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 30*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 35*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 40*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 45*/ node_sub2     , node_sub2     , node_sub2     , node_num1     , node_num1     ,
        /* 50*/ node_num1     , node_num1     , node_num1     , node_num1     , node_num1     ,
        /* 55*/ node_num1     , node_num1     , node_num1     , node_sub2     , node_sub2     ,
        /* 60*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 65*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 70*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 75*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 80*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 85*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 90*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /* 95*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*100*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*105*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*110*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*115*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*120*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*125*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*130*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*135*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*140*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*145*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*150*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*155*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*160*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*165*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*170*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*175*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*180*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*185*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*190*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*195*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*200*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*205*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*210*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*215*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*220*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*225*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*230*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*235*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*240*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*245*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*250*/ node_sub2     , node_sub2     , node_sub2     , node_sub2     , node_sub2     ,
        /*255*/ node_sub2
    },
    /*state_var*/
    {
        /*  0*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /*  5*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 10*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 15*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 20*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 25*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 30*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 35*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 40*/ node_var      , node_var      , node_var      , node_var      , node_var      ,
        /* 45*/ node_var1     , node_var1     , node_var      , node_var1     , node_var1     ,
        /* 50*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /* 55*/ node_var1     , node_var1     , node_var1     , node_var      , node_var      ,
        /* 60*/ node_var      , node_var      , node_var      , node_var      , node_var1     ,
        /* 65*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /* 70*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /* 75*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /* 80*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /* 85*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /* 90*/ node_var1     , node_var1     , node_var      , node_var1     , node_var      ,
        /* 95*/ node_var1     , node_var      , node_var1     , node_var1     , node_var1     ,
        /*100*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*105*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*110*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*115*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*120*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var      ,
        /*125*/ node_var1     , node_var      , node_var      , node_var1     , node_var1     ,
        /*130*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*135*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*140*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*145*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*150*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*155*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*160*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*165*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*170*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*175*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*180*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*185*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*190*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*195*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*200*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*205*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*210*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*215*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*220*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*225*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*230*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*235*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*240*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*245*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*250*/ node_var1     , node_var1     , node_var1     , node_var1     , node_var1     ,
        /*255*/ node_var1
    }
};

int xcontext_get_token(xcontext_t context) {
    char * token_bg;
    uint32_t cur_node;
    uint32_t prv_node;
    int i;
    xtoken_t token;

    assert(context->m_cur_token == NULL);
    context->m_cur_token = xcomputer_alloc_token(context->m_computer);
    if (context->m_cur_token == NULL) {
        CPE_ERROR(
            context->m_computer->m_em, "xcontext_get_token: alloc token fail!");
        return -1;
    }

    token = context->m_cur_token;

    token_bg = context->m_cur_pos;
    cur_node = __make_node(state_begin, XTOKEN_END);

    for(i = 0; i < 50000; ++i) {
		uint32_t cur_node_state;
        prv_node = cur_node;
        cur_node_state = __state(cur_node);
        assert(cur_node_state < __state_num);

        cur_node = node_table[cur_node_state][(uint8_t)(*context->m_cur_pos++)];

        if (__is_end(cur_node)) {
            uint32_t cur_token_type = __token(cur_node);
            if (xtoken_type_index(cur_token_type) >= XTOKEN_ERROR) goto SCAN_ERROR;

            token->m_type = cur_token_type;
            if (xtoken_type_is_data(cur_token_type)) {
                if (cur_token_type == XTOKEN_NUM_INT) {
                    char buf[32];
                    xtoken_set_int(token, atoi(cpe_str_dup_range(buf, sizeof(buf), token_bg, context->m_cur_pos)));
                }
                else if (cur_token_type == XTOKEN_NUM_FLOAT) {
                    char buf[32];
                    xtoken_set_double(token, atof(cpe_str_dup_range(buf, sizeof(buf), token_bg, context->m_cur_pos)));
                }
                else if (cur_token_type == XTOKEN_STRING) {
                    while(*token_bg == ' ' || *token_bg == '\n' || *token_bg == '\r') token_bg++;
                    if(*token_bg == '"' || *token_bg == '\'') {
                        xtoken_set_str(token, token_bg + 1, context->m_cur_pos - 1);
                    }
                    else {
                        xtoken_set_str(token, token_bg, context->m_cur_pos - 1);
                    }
                }
                else {
                    assert(cur_token_type == XTOKEN_VAL);
                    while(*token_bg == ' ' || *token_bg == '\t') token_bg++;
                    xtoken_set_str(token, token_bg + 1, context->m_cur_pos - 1);
                }
            }
            else if (cur_token_type == XTOKEN_FUNC) {
                while(*token_bg == ' ' || *token_bg == '\t') token_bg++;
                xtoken_set_str(token, token_bg, context->m_cur_pos - 1);
            }

            if (__need_back(cur_node)) {
                context->m_cur_pos--;
            }

            return 0;
        }
    }

SCAN_ERROR:
    assert(context->m_cur_token);
    xcomputer_free_token(context->m_computer, context->m_cur_token);
    context->m_cur_token = NULL;

    if (context->m_computer->m_em) {
        char parsed_buf[128];
        char current_buf[128];

        if (token_bg - context->m_buf + 1 > sizeof(parsed_buf)) {
            memcpy(parsed_buf, token_bg - sizeof(parsed_buf) + 1, sizeof(parsed_buf) - 1);
            parsed_buf[sizeof(parsed_buf) - 1] = 0;
        }
        else {
            memcpy(parsed_buf, context->m_buf, token_bg - context->m_buf);
            parsed_buf[token_bg - context->m_buf] = 0;
        }

        if (context->m_cur_pos - token_bg + 1 > sizeof(current_buf)) {
            memcpy(current_buf, context->m_cur_pos - sizeof(current_buf) + 1, sizeof(current_buf) - 1);
            current_buf[sizeof(current_buf) - 1] = 0;
        }
        else {
            memcpy(current_buf, token_bg, context->m_cur_pos - token_bg);
            current_buf[context->m_cur_pos - token_bg] = 0;
        }

        CPE_ERROR(
            context->m_computer->m_em,
            "lex error\n"
            "    parsed  -> <%s>\n"
            "    current -> <%s>\n"
            "    pos     -> <%d:%d>\n"
            "    pre state -> <%d:%d:%d>\n"
            "    cur state -> <%d:%d:%d>\n"
            ,
            parsed_buf, current_buf
            , (int)(token_bg - context->m_buf), (int)(context->m_cur_pos - token_bg)
            , __state(prv_node), xtoken_type_index(__token(prv_node)), (xtoken_type_is_data(__token(prv_node)) ? 1 : 0)
            , __state(cur_node), xtoken_type_index(__token(cur_node)), (xtoken_type_is_data(__token(cur_node)) ? 1 : 0)
            );
    }

    return -1;
}

xcontext_t xcontext_create(xcomputer_t computer, const char * str) {
    int len;
	xcontext_t context;

    len = (int)strlen(str) + 1;
    
    context = mem_alloc(computer->m_alloc, sizeof(struct xcontext) + len);
    if (context == NULL) return NULL;

    context->m_computer = computer;
    context->m_buf = (char*)(context + 1);
    context->m_cur_pos = context->m_buf;
    context->m_cur_token = NULL;
    TAILQ_INIT(&context->m_tokens);
    context->m_sign_token = NULL;

    memcpy(context->m_buf, str, len);

    return context;
}

void xcontext_free(xcontext_t context) {
    if (context->m_cur_token) {
        xcomputer_free_token(context->m_computer, context->m_cur_token);
        context->m_cur_token = NULL;
    }

    while(!TAILQ_EMPTY(&context->m_tokens)) {
        xcomputer_free_token(context->m_computer, xcontext_pop_token(context));
    }

    mem_free(context->m_computer->m_alloc, context);
}

void xcontext_push_token(xcontext_t context) {
    assert(context->m_cur_token);
    TAILQ_INSERT_HEAD(&context->m_tokens, context->m_cur_token, m_next);

    if (xtoken_is_sign(context->m_cur_token)) {
        context->m_sign_token = context->m_cur_token;
    }

    context->m_cur_token = NULL;
}

xtoken_t xcontext_pop_token(xcontext_t context) {
    xtoken_t r;

    assert(!TAILQ_EMPTY(&context->m_tokens));

    r = TAILQ_FIRST(&context->m_tokens);

    TAILQ_REMOVE(&context->m_tokens, r, m_next);

    if (context->m_sign_token == r) {
        TAILQ_FOREACH(context->m_sign_token, &context->m_tokens, m_next) {
            if (xtoken_is_sign(context->m_sign_token)) break;
        }
    }

    return r;
}

void xcontext_update_sign_token(xcontext_t context) {
    TAILQ_FOREACH(context->m_sign_token, &context->m_tokens, m_next) {
        if (xtoken_is_sign(context->m_sign_token)) break;
    }
}

xtoken_t xcontext_token_n(xcontext_t context, int n) {
    xtoken_t r = TAILQ_FIRST(&context->m_tokens);

    while(r && n > 0) {
        r = TAILQ_NEXT(r, m_next);
    }

    return n == 0 ? r : NULL;
}

void xcontext_dump_stack(xcontext_t context) {
    xtoken_t token;
    struct mem_buffer buffer;
    struct write_stream_buffer s = CPE_WRITE_STREAM_BUFFER_INITIALIZER(&buffer);

    mem_buffer_init(&buffer, context->m_computer->m_alloc);

    stream_printf((write_stream_t)&s, "=== dump stack");

    TAILQ_FOREACH(token, &context->m_tokens, m_next) {
        xtoken_t sub_token;

        stream_putc((write_stream_t)&s, '\n');
        stream_putc_count((write_stream_t)&s, ' ', 4);

        xtoken_dump((write_stream_t)&s, token);

        for(sub_token = token->m_sub; sub_token; sub_token = sub_token->m_sub) {
            stream_printf((write_stream_t)&s, " ==> ");
            xtoken_dump((write_stream_t)&s, sub_token);
        }
    }

    stream_putc((write_stream_t)&s, 0);

    CPE_INFO(
        context->m_computer->m_em, "%s", 
        (const char *)mem_buffer_make_continuous(&buffer, 0));

    mem_buffer_clear(&buffer);
}
