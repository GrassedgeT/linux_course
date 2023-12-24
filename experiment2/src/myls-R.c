#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

void print_file_info(const char *file_name) {
    struct stat file_stat;
    if (stat(file_name, &file_stat) == -1) {
        perror("stat");
        return;
    }
    printf("%-20s %10ld\t%s", file_name, file_stat.st_size, ctime(&file_stat.st_mtime));
}

void list_dir(const char *dir_name) {
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    //打印表头
    printf("%-25s %10s\t%s\n", "文件名", "大小", "最后修改时间");  

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        print_file_info(entry->d_name);
    }

    closedir(dir);
}

void list_dir_recursive(const char *dir_name){
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("opendir");
        return;
    }


    printf("%s:\n", dir_name);
    printf("%-25s %10s %s\n", "文件名", "大小", "最后修改时间");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        print_file_info(entry->d_name);


        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char sub_dir_name[PATH_MAX];
            snprintf(sub_dir_name, sizeof(sub_dir_name), "%s/%s", dir_name, entry->d_name);
            list_dir_recursive(sub_dir_name);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        list_dir(".");
    } else {
        if (strcmp(argv[1], "-l") == 0) {
            if (argc == 2) {
                list_dir(".");
            } else {
                if (chdir(argv[2]) != 0) {
                    perror("chdir");
                    return 1;
                }
                list_dir(".");
            }
        } else if (strcmp(argv[1], "-R") == 0) {
            if (argc == 2) {
                list_dir_recursive(".");
            } else {
                if (chdir(argv[2]) != 0) {
                    perror("chdir");
                    return 1;
                }
                list_dir_recursive(".");
            }
        } else {
            if (chdir(argv[1]) != 0) {
                perror("chdir");
                return 1;
            }
            list_dir(".");
        }
    }

    return 0;
}