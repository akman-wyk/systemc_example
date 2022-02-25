#pragma once
#include <systemc.h>
#include "config.h"

SC_MODULE(vector_mul) {

    sc_in<bool> clk,rst_n;
    sc_in<sc_int<WIDTH> > vec1[VEC_WIDTH],vec2[VEC_WIDTH];
    sc_out<sc_int<WIDTH * 2> > vec_o;

    void compute_vector_mul();

    SC_CTOR(vector_mul) {
        SC_METHOD(compute_vector_mul);
        sensitive_pos << clk;
        sensitive_neg << rst_n;
    }
};