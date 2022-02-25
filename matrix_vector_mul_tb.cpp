#include <systemc.h>
#include <sstream>
#include "matrix_vector_mul.h"
#include "driver.h"
#include "config.h"

int sc_main(int argc,char* argv[]) {
    sc_clock clk("clk",10,SC_NS);
    sc_signal<bool> rst_n;
    sc_signal<sc_int<WIDTH> > mat[VEC_NUM][VEC_WIDTH],vec[VEC_WIDTH];
    sc_signal<sc_int<WIDTH * 2> >vec_o[VEC_NUM];

    sc_trace_file *fp;                  // Create VCD file
    fp=sc_create_vcd_trace_file("wave");// open(fp), create wave.vcd file
    fp->set_time_unit(1, SC_NS);        // set tracing resolution to ns

    matrix_vector_mul dut("dut");
    dut.clk(clk);
    dut.rst_n(rst_n);
    for (int i = 0; i < VEC_NUM; ++i) {
        for (int j = 0; j < VEC_WIDTH; ++j) {
            dut.matrix[i][j](mat[i][j]);
        }
    }
    for (int i = 0; i < VEC_WIDTH; ++i) {
        dut.vector_in[i](vec[i]);
    }
    for (int i = 0; i < VEC_NUM; ++i) {
        dut.vector_out[i](vec_o[i]);
    }

    driver d("dri");
    d.clk(clk);
    d.rst_n(rst_n);
    for (int i = 0; i < VEC_WIDTH; ++i) {
        for (int j = 0; j < VEC_NUM; ++j) {
            d.mat[j][i](mat[j][i]);
        }
        d.vec[i](vec[i]);
    }

    sc_trace(fp,clk,"clk");
    sc_trace(fp,rst_n,"rst_n");
    for (int i = 0; i < VEC_NUM; ++i) {
        for (int j = 0; j < VEC_WIDTH; ++j) {
            std::ostringstream mat_name;
            mat_name << "matrix(" << i << "," << j << ")";
            sc_trace(fp,mat[i][j],mat_name.str());
            mat_name.str("");
        }
    }
    for (int i = 0; i < VEC_WIDTH; ++i) {
        std::ostringstream stream1;
        stream1 << "vec(" << i << ")";
        sc_trace(fp,vec[i],stream1.str());
        stream1.str("");
    }
    for (int i = 0; i < VEC_NUM; ++i) {
        std::ostringstream out_name;
        out_name << "dout(" << i << ")";
        sc_trace(fp,vec_o[i],out_name.str());
        out_name.str("");
    }

    sc_start(1000,SC_NS);

    sc_close_vcd_trace_file(fp);        // close(fp)
    return 0;
}