#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

//添加随机后缀的函数
char* add_random_suffix(const char* str, int random_number) {

    char* new_str = (char*)malloc(strlen(str) + 5); // 分配足够的内存来存储新的字符串
    
    if (new_str == NULL) {
        return NULL; // 内存分配失败
    }

    sprintf(new_str, "%s%04d", str, random_number); // 格式化新的字符串

    return new_str;
}

//生成指定范围随即整数的函数
int random_int(int min, int max) {
    srand(time(NULL)); // 初始化随机数生成器
    return rand() % (max - min + 1) + min; // 生成一个指定范围的随机数
}