/*
    查找目录树中具有特定名称的所有文件
*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"    // 包含 struct dirent 等定义

// find函数：递归查找
void find(char *path, char *target_name) {
    char buf[512], *p;  // buf 保存当前拼接的路径  p 指向buf的路径末尾 用于拼接新文件名
    int fd;  // 文件描述符，表示打开的目录
    struct dirent de;  // 目录项结构体，包含文件名(char name[DIRSIZ])和 inode编号(inum)
    struct stat st;  // 文件状态结构体，包含文件类型、大小等信息

    // 打开路径  以只读模式打开一个文件或目录  
    if ((fd = open(path, 0)) < 0) {  // fd 是你用 open() 打开的一个目录（或文件），是一个整数
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // 获取 stat，判断是不是目录
    if (fstat(fd, &st) < 0) {  // 获取打开文件对应的详细状态信息
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // 如果不是目录，则关闭  因为 find() 是设计用来“进入目录”的
    if (st.type != T_DIR) {  // 是目录则不会进这个if
        close(fd);
        return;
    }

    // 拼接路径前缀 防止路径过长
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {  // DIRSIZ=14 在宏定义中定义的
        printf("find: path too long\n");
        close(fd);
        return;
    }

    strcpy(buf, path);  // 复制路径到 buf（缓冲区）
    p = buf + strlen(buf);  // p 指向 buf 的末尾
    *p++ = '/';  // 添加路径分隔符

    // 读取目录项
    while (read(fd, &de, sizeof(de)) == sizeof(de)) { // read() 每次返回读到的字节数
        // 忽略空项
        if (de.inum == 0)
            continue;

        // 将目录项的文件名拷贝到 buf 的末尾
        memmove(p, de.name, DIRSIZ);  // 把 de.name 中最多 14 字节的文件名复制到 p 所指向的路径字符串末尾
        p[DIRSIZ] = 0;  // 确保 buf 以 null 结尾 

        // 获取该项的状态信息
        if (stat(buf, &st) < 0) {  // 用 stat() 读取拼接后的路径
            printf("find: cannot stat %s\n", buf);
            continue;
        }

        // 核心判断逻辑 
        if (st.type == T_DIR) {  // 是目录，则递归
            // 跳过 . 和 ..
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
                continue;
            // 递归进入子目录
            find(buf, target_name);
        } else {  // 是文件，名字匹配就输出
            if (strcmp(de.name, target_name) == 0) {
                printf("%s\n", buf);
            }
        }
    }
    // UNIX 的find本身也如此: 找不到不会报错，只是不输出
    close(fd);  // 关闭目录文件描述符
}

int main(int argc, char *argv[]) {
    if (argc != 3) {  // 期望参数数量为 3  程序名 + 起始路径 + 文件名
        fprintf(2, "Usage: find <directory> <filename>\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}
