cmake_minimum_required(VERSION 3.8.2)
project(CS4075 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

add_executable(CS4075 src/hello-from-processes/main.c src/homework-0/mpi-vector.c src/homework-1/integral.c src/homework-1/ src/homework-2 src/homework-2/prefix.c src/homework-3/matrix.c src/homework-3/matrix-test.c src/homework-3/test.c src/homework-3/calcpi.c src/pthread-hello.c)