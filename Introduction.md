### 简介

该系统由以下几个部分组成：

+ 乘法器：
  + 向量-向量乘法器（vector_mul）：实现向量乘法功能
  + 矩阵-向量乘法器（matrix_vector_mul）：实现矩阵-向量乘法功能，由多个向量-向量乘法器组成
+ 测试部分
  + 数据生成器（driver）：生成输入数据
  + 主程序（matrix_vector_mul_tb）：实例化上述模块，进行仿真



### vector_mul

SystemC 中使用类来描述模块，使用如下宏定义：

```c++
//vector_mul.h
SC_MODULE(vector_mul) {
	......
};
```

上述代码实际上定义了一个命名为 vector_mul 的类，并继承了 SystemC 中的 sc_module 类。

模块中至少需要包含三部分内容：端口和信号定义，功能描述，构造函数

##### 1. 端口和信号定义

SystemC 的模块通过定义类成员变量的方式定义端口和信号，如下所示：

```c++
//vector_mul.h
sc_in<bool> clk,rst_n;
sc_in<sc_int<WIDTH> > vec1[VEC_WIDTH],vec2[VEC_WIDTH];
sc_out<sc_int<WIDTH * 2> > vec_o;
```

端口即模块间通信的端口，分为 in, out, inout 三种类型，分别用模板 `sc_in<>`, `sc_out<>`, `sc_inout<>` 定义。信号描述了模块内部进程间的通信，即连线，用模板 `sc_signal<>` 定义。上述模板的类型参数为端口和信号的数据类型，可以使用 c++ 内置数据类型，如 `bool` 等，也可以使用 SystemC 定义的基本数据类型，如 `sc_int<>`（即有符号整型，需要给出数据位宽）

除了端口和信号外，也可定义普通的类成员变量。

##### 2. 功能描述

SystemC 的模块通过定义无输入参数、无输出参数的方法来描述模块功能，如下所示。一个模块可以有多个功能描述，类似于 Verilog 中的 `always` 块。

```c++
//vector_mul.h
void compute_vector_mul();

//vector_mul.cpp
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
```

该部分可以使用 c++ 的特性和库。其中对于端口和信号的读写，推荐使用 `.read()` 和 `.write()` 方法，这两个方法可以帮助完成可能需要的隐式类型转换，避免不必要的错误产生。此外，SystemC 中的赋值基本为阻塞赋值。

除了直接描述模块功能的无参void方法外，也可以定义其他普通类方法以辅助完成功能描述。

##### 3. 构造函数

SystemC 模块的构造函数可以用来注册功能并描述功能敏感列表，如下所示：

```c++
//vector_mul.h
SC_CTOR(vector_mul) {
    SC_METHOD(compute_vector_mul);
    sensitive_pos << clk;
    sensitive_neg << rst_n;
}
```

无参构造函数使用宏 `SC_CTOR(module_name)` 定义，其中的 module_name 需要与模块名一致。

类中可能有很多方法，只有通过注册后的方法才能作为模块的功能，此处通过宏 `SC_METHOD` 将方法 `compute_vector_mul` 注册为 `METHOD` 功能。`METHOD` 是一种阻塞式的功能进程，当这个进程被敏感列表触发之后，获取仿真控制权开始运行，直到运行完成，将控制权返回SystemC仿真内核。使用`METHOD`注册的功能函数不能含有无限循环，这会导致仿真卡死在这个任务中，控制权无法返回仿真内核。

注册完功能后需要立即描述该功能的敏感列表，类似  Verilog 中的 `always` 块的敏感列表，只有敏感列表中的变量发生改变时，功能才能运行。其中 `sensitive <<` 表示事件（电平）敏感，一般用于组合逻辑建模；`sensitive_pos <<` 和 `sensitive_neg <<` 分别为正跳变敏感和负跳变敏感，一般用于时序逻辑建模。

上述三部分内容全部定义完成后，完成对模块vector_mul的构建。



### matrix_vector_mul

该模块为上层模块，由多个 vector_mul 模块组成。因此需要额外完成子模块声明、实例化和端口连接。

子模块声明如下所示，此处声明为指针变量，也可声明为普通变量。

```c++
vector_mul *pe[VEC_NUM];
```

子模块实例化和端口连接在构造函数中完成。子模块实例化实质上是调用了子模块的构造函数，若调用了宏 `SC_CTOR` 定义的构造函数，则需要给出一个唯一的字符串作为实例名，如下所示：

```c++
pe_name << "pe" << i;
pe[i] = new vector_mul(pe_name.str().c_str());
```

端口连接采用了 `submodule_ptr->port_name(signal_name)`，即名字关联。也可采用位置关联，即按照端口定义的顺序一一对应连接。但位置关联可能产生连接顺序错误，推荐使用名字关联。

```c++
pe[i]->clk(clk);
pe[i]->rst_n(rst_n);
for (int j = 0; j < VEC_WIDTH; ++j) {
    pe[i]->vec1[j](matrix[i][j]);
    pe[i]->vec2[j](vector_in[j]);
}
pe[i]->vec_o(vector_out[i]);
```

模块中也可以定义析构函数，用于释放资源：

```c++
~matrix_vector_mul() {
    for (int i = 0; i < VEC_NUM; ++i) {
        if (pe[i] != nullptr) {
            delete pe[i];
            pe[i] = nullptr;
        }
    }
}
```



### driver

该模块产生输入数据，同样由端口和信号定义，功能描述，构造函数三部分组成。

该模块的构造函数中将方法 `generate_input` 和 `generate_reset` 注册为 `THREAD` 功能，如下所示：

```c++
//driver.h
SC_CTOR(driver) {
    SC_THREAD(generate_input);
    sensitive_neg << clk;
    SC_THREAD(generate_reset);
};
```

这种功能进程在仿真开始时运行，碰到`wait()`跳出，直到敏感列表中的信号再次触发这一进程，从上次跳出的 `wait()` 处继续运行，因此这种进程可以使用循环体包括 `wait()` 的无限循环。`THREAD` 功能一般用于测试中的生成输入数据与获取输出数据，也适用于一些需要保存中间结果的功能。

除了使用`wait()`阻塞运行外，还可以使用 `wait(<times>,SC_NS)`，将执行延迟指定的时钟周期，如`rst_n`信号的实现，使用多个 `wait(<times>,SC_NS)` 延迟执行。

```c++
//driver.cpp
void driver::generate_reset() {
    rst_n.write(1);
    wait(1,SC_NS);
    rst_n.write(0);
    wait(1,SC_NS);
    rst_n.write(1);
}
```



### matrix_vector_mul_tb

SystemC 的程序入口为 `sc_main` 函数，分为以下几个部分。

第一部分为信号声明，声明连接需要的信号。注意时钟信号需要声明为专门的类型，并给出相应参数，此处生成了一个周期为10ns的时钟：

```c++
sc_clock clk("clk",10,SC_NS);
sc_signal<bool> rst_n;
sc_signal<sc_int<WIDTH> > mat[VEC_NUM][VEC_WIDTH],vec[VEC_WIDTH];
sc_signal<sc_int<WIDTH * 2> >vec_o[VEC_NUM];
```

第二部分为声明并实例化模块，以及连接信号，与之前相似，此处实例化了 matrix_vector_mul 和 driver 两个模块：

```c++
//matrix_vector_mul
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

//driver
driver d("dri");
d.clk(clk);
d.rst_n(rst_n);
for (int i = 0; i < VEC_WIDTH; ++i) {
    for (int j = 0; j < VEC_NUM; ++j) {
        d.mat[j][i](mat[j][i]);
    }
    d.vec[i](vec[i]);
}
```

第三部分为波形跟踪和保存，首先是波形文件的生成和定义：

```c++
sc_trace_file *fp;                  	// 创建文件指针
fp=sc_create_vcd_trace_file("wave");	// 生成文件，并命名为wave.vcd
fp->set_time_unit(1, SC_NS);   			// 设置方针时间单位为1ns
```

然后是使用 `sc_trace(fp, signal, signal_name)` 将需要观察的信号添加到波形跟踪中，其中 signal_name 为波形文件中信号的名称，需要保证唯一。如下所示：

```c++
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
```

最后仿真结束后，需要关闭仿真文件：

```c++
sc_close_vcd_trace_file(fp);
```

最后一部分是启动和结束仿真，使用 `sc_start()` 启动仿真并指定仿真时长：

```c++
sc_start(1000,SC_NS);
```

上例中仿真将执行到1000ns结束或遇到 `sc_stop()`。

也可以不指定仿真时长，此时仿真将一直执行直到遇到 `sc_stop()`。