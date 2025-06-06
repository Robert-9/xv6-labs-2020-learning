/*
从标准输入中按行读取，并且为每一行执行一个命令，将行作为参数提供给命令
main的逻辑：
    1.把用户命令（比如 echo hello）先保存起来；
    2.然后从标准输入一字一字读取内容；
    3.遇到空格就认为是“一个参数”，保存；
    4.遇到换行就认为“读完了一组参数”，于是：
        把这一行的参数加在之前保存的命令后面；
        执行这条命令一次；
*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void run(char *program, char **args) {
    // 创建子进程执行命令
    if (fork() == 0) {
        exec(program, args); // 用给定参数执行
        exit(0);             // 如果 exec 出错，还是要退出
    }
    // 父进程返回（不等待，这由 main 最后统一 wait）
    return;
}

int main(int argc, char *argv[]) {
    char buf[2048];              // 输入缓冲区
    char *p = buf, *last_p = buf;
    char *argsbuf[128];          // 参数数组
    char **args = argsbuf;

    // 复制 argv[1:] 到 argsbuf 开头  (0 是xargs)
    for (int i = 1; i < argc; i++) {
        *args = argv[i];
        args++;
    }

    char **pa = args;            // 当前追加参数的位置

    // 从标准输入读入字符（逐字节读取）
    while (read(0, p, 1) != 0) {
        // 空格或换行：表示参数结束
        if (*p == ' ' || *p == '\n') {
            *p = '\0';         // 替换为 C 字符串结束符  这里就是字符串的结尾
            // 把参数加入 argsbuf  pa是每次追加的起始位置，后面都做了重置的
            *(pa++) = last_p;  // last_p 是当前字符串的起点，它会自动将前面添加的'\0'作为终点划分字符串
            last_p = p + 1;    // 准备下一个参数起点
        }

        // 如果是换行：说明一组参数读完了，可以执行命令了
        if (*p == '\n') {
            *pa = 0;                  // NULL 结尾
            run(argv[1], argsbuf);    // 执行命令
            pa = args;                // 参数指针重置
        }

        p++;  // 移动读取指针
    }

    // 如果最后一行没有换行，但仍有参数，也要处理一次
    if (pa != args) {
        *p = '\0';
        *(pa++) = last_p;
        *pa = 0;
        run(argv[1], argsbuf);
    }

    // 等待所有子进程执行完
    while (wait(0) != -1) {}

    exit(0);
}
