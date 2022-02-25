#include "driver.h"

void driver::generate_input() {
    for (int i = 0; i < VEC_WIDTH; ++i) {
        for (int j = 0; j < VEC_NUM; ++j) {
            mat[j][i].write(rand() % ((int)pow(2,WIDTH) - 1));
        }
        vec[i].write(rand() % ((int)pow(2,WIDTH) - 1));
    }
    while(1) {
        wait();
        for (int i = 0; i < VEC_WIDTH; ++i) {
            for (int j = 0; j < VEC_NUM; ++j) {
                mat[j][i].write(rand() % ((int)pow(2,WIDTH) - 1));
            }
            vec[i].write(rand() % ((int)pow(2,WIDTH) - 1));
        }
    }
}

void driver::generate_reset() {
    rst_n.write(1);
    wait(1,SC_NS);
    rst_n.write(0);
    wait(1,SC_NS);
    rst_n.write(1);
}