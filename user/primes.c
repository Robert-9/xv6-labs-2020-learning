// 这个程序实现了一个简单的素数筛选器，使用管道和递归来筛选2到35之间的素数。
// 它通过创建一个管道，将数据从父进程传递到子进程，然后子进程继续筛选并将结果传递给下一个子进程，直到所有素数都被筛选出来。

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 每个进程运行的 primes 筛选逻辑
void primes(int p_read) {
    int prime;
    if (read(p_read, &prime, sizeof(int)) != sizeof(int)) {
        // 没读到任何数，说明没有数据了
        close(p_read);
        exit(0);
    }

    printf("prime %d\n", prime);

    int p_next[2]; 
    pipe(p_next);  // 创建下一个管道，用于传递筛选后的数

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // 子进程：从 p_next[0] 读（下游）
        close(p_next[1]);     // 不写
        close(p_read);        // 不再读上游
        primes(p_next[0]);    // 递归启动下一个 primes
    } else {
        // 父进程：筛选并写入到 p_next[1]
        int num;
        while (read(p_read, &num, sizeof(int)) == sizeof(int)) {
            if (num % prime != 0) {  // 等于0则不是素数
                write(p_next[1], &num, sizeof(int));
            }
        }

        close(p_read);        // 读完，关闭管道（关闭本进程的读，本进程的写在其父进程关的）
        close(p_next[1]);     // 写完，关闭管道（关闭子进程的写）
        wait(0);              // 等待子进程退出
        exit(0);              // 自己也退出
    }
}

int main(int argc, char *argv[]) {
    int p[2];  // 0读 1写
    pipe(p);

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed\n");
        exit(1);
    }

    if (pid == 0) {
        // 子进程：进入 primes 链
        close(p[1]);  // 不写，读取即可（一个进程只保留自己需要的文件描述符）
        primes(p[0]);
    } else {
        // 父进程：写入 2~35 到管道
        close(p[0]);  // 不读，只写
        for (int i = 2; i <= 35; i++) {
            write(p[1], &i, sizeof(int));
        }
        close(p[1]);  // 写完后必须关闭，让子进程读取到 EOF
        wait(0);      // 等待 primes 链全部结束
    }

    exit(0);
}

