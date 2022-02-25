#pragma once
#include <systemc.h>
#include "config.h"

SC_MODULE(driver)
{
    sc_in <bool> clk;
    sc_out<bool> rst_n;
    sc_out<sc_int<WIDTH> > mat[VEC_NUM][VEC_WIDTH];
    sc_out<sc_int<WIDTH> > vec[VEC_WIDTH];

    void generate_input();
    void generate_reset();

    SC_CTOR(driver) {
        SC_THREAD(generate_input);
        sensitive_neg << clk;
        SC_THREAD(generate_reset);
    };
};