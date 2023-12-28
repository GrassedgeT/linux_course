#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//添加随机后缀的函数
char* add_random_suffix(const char* str) {
    srand(time(NULL)); // 初始化随机数生成器
    int random_number = rand() % 10000; // 生成一个四位随机数

    char* new_str = (char*)malloc(strlen(str) + 5); // 分配足够的内存来存储新的字符串
    if (new_str == NULL) {
        return NULL; // 内存分配失败
    }

    sprintf(new_str, "%s%04d", str, random_number); // 格式化新的字符串

    return new_str;
}