#include "vector_mul.h"

void vector_mul::compute_vector_mul() {
    int temp = 0;
    if (rst_n.read() == false) {
        vec_o.write(0);
        return;
    }
    for (int i = 0; i < VEC_WIDTH; ++i) {
        temp = temp + vec1[i].read() * vec2[i].read();
    }
    vec_o.write(temp);
}