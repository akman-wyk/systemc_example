CC     = g++
OPT    = -O2 # -O3
DEBUG  = -g
OTHER  = -Wall -Wno-deprecated
CFLAGS = $(OPT) $(OTHER)
SYSTEMC = /home/wyk/systemc
INCDIR = -I$(SYSTEMC)/include
LIBDIR = -L$(SYSTEMC)/lib-linux64
LIBS = -lsystemc
APP=test.o
SRCS=vector_mul.cpp driver.cpp matrix_vector_mul_tb.cpp
all:
	g++ -o $(APP) $(SRCS) $(LIBDIR) $(INCDIR) $(LIBS)
clean:
	rm -rf *.o
