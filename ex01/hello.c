#include <stdio.h>

// 自定义输入函数：从标准输入读取一行，最多读取 size-1 个字符，并存入 str
void input(char *str, int size) {
    int i = 0;
    char c;
    
    // 逐个读取字符，直到遇到换行符或达到缓冲区上限
    while (i < size - 1) {
        c = getchar();           // 读取一个字符
        if (c == '\n' || c == EOF) {
            break;                // 遇到换行或文件结束符，停止读取
        }
        str[i++] = c;             // 将字符存入数组
    }
    str[i] = '\0';                // 添加字符串结束符

    // 如果输入过长，还需清除缓冲区中剩余字符（可选，这里简化处理）
    if (c != '\n' && c != EOF) {
        while (getchar() != '\n'); // 丢弃后续字符直至换行
    }
}

int main() {
    printf("Hello World!\n");
    char buffer[100];             // 定义字符数组存放输入

    printf("请输入内容：");
    input(buffer, sizeof(buffer)); // 调用自定义 input 函数

    printf("您输入的内容是：%s\n", buffer);

    return 0;
}
