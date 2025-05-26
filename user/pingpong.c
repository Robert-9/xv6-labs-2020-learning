// user/pingpong.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    //pp2c用于父到子进程的通信（父写子读），c2pp用于子到父进程的通信（子写父读）    int pp2c[2], c2pp[2];
    int pp2c[2], c2pp[2];
    if (pipe(pp2c) < 0 || pipe(c2pp) < 0) {  // pipe()单向管道。入参是长度为2的整型数组 fd[2]。[0]是读，[1]是写。
        fprintf(2, "pingpong: pipe creation failed\n");
        exit(1);
    }

    // fork()是一个 创建子进程的系统调用。调用后，当前进程会被“复制一份”成子进程。
    // 返回值：子进程中返回 0；父进程中返回子进程的 PID；出错返回负数。
    int pid = fork(); 
    if (pid < 0) {
        fprintf(2, "pingpong: fork failed\n");
        exit(1);
    }
    if (pid == 0) { // 子进程
        close(pp2c[1]); // 关闭父进程到子进程的写端
        close(c2pp[0]); // 关闭子进程到父进程的读端

        char buf[1];
        while (read(pp2c[0], buf, 1) > 0) { // 从父进程读取数据
            write(1, "received pong\n", 14); // 输出"pong"
            write(c2pp[1], buf, 1); // 将数据写回父进程
        }
        close(pp2c[0]);
        close(c2pp[1]);
        exit(0);
    } else { // 父进程
        close(pp2c[0]); // 关闭父进程到子进程的读端
        close(c2pp[1]); // 关闭子进程到父进程的写端

        char buf[1] = {'p'};
        write(pp2c[1], buf, 1); // 向子进程发送数据
        read(c2pp[0], buf, 1); // 等待子进程响应

        write(1, "ping\n", 5); // 输出"ping"
        
        close(pp2c[1]);
        close(c2pp[0]);
        wait(0); // 等待子进程结束
    }
    exit(0);
}