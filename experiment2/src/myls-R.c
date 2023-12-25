#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <linux/limits.h>

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
    if (chdir(dir_name) != 0) {
        perror("chdir");
        return 1;
    }

    print_cwd();
    printf("%-25s %10s\t%s\n", "文件名", "大小", "最后修改时间");  

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        print_file_info(entry->d_name);
    }

    closedir(dir);
    if (chdir("..") != 0) {
        perror("chdir");
        return 1;
    }
}

void list_dir_recursive(const char *dir_name){
    
    list_dir(dir_name);
    DIR *dir = opendir(dir_name);
    if (dir == NULL) {
        perror("opendir");
        return;
    }
    if (chdir(dir_name) != 0) {
        perror("chdir");
        return 1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
    
        if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            list_dir_recursive(entry->d_name);
        }
    }

    closedir(dir);
    if (chdir("..") != 0) {
        perror("chdir");
        return 1;
    }
}

void print_cwd(){
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("\n%s\n", cwd);
    } else {
        perror("getcwd");
        return 1;
    }

}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        list_dir(".");
    } else {
        if (strcmp(argv[1], "-l") == 0) {
            if (argc == 2) {
                list_dir(".");
            } else {
                // if (chdir(argv[2]) != 0) {
                //     perror("chdir");
                //     return 1;
                // }
                
                list_dir(argv[2]);
            }
        } else if (strcmp(argv[1], "-R") == 0) {
            if (argc == 2) {
                list_dir_recursive(".");
            } else {
                // if (chdir(argv[2]) != 0) {
                //     perror("chdir");
                //     return 1;
                // }
                list_dir_recursive(argv[2]);
            }
        } else {
            // if (chdir(argv[1]) != 0) {
            //     perror("chdir");
            //     return 1;
            // }
            list_dir(argv[1]);
        }
    }

    return 0;
}