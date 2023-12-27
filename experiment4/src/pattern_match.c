#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>

#define MAX_FILES UINT16_MAX
#define MAX_FILENAME 100
#define NUM_CHILDREN 10

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s keyword\n", argv[0]);
        exit(1);
    }

    char *keyword = argv[1];
    char files[MAX_FILES][MAX_FILENAME];
    int num_files = 0;

    DIR *dir = opendir("/usr/include");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        strncpy(files[num_files], entry->d_name, MAX_FILENAME);
        num_files++;
        if (num_files >= MAX_FILES) break;
    }
    closedir(dir);

    int pipefd[2];
    pipe(pipefd);

    for (int i = 0; i < NUM_CHILDREN; i++) {
        if (fork() == 0) {  
            int pid = getpid();
            // 子进程
            if(chdir("/usr/include") == -1) {
                perror("chdir failed");
                exit(1);
            }
            close(pipefd[0]);  // 关闭管道的读端
            for (int j = i * num_files / NUM_CHILDREN; j < (i+1) * num_files / NUM_CHILDREN; j++) {
                FILE *file = fopen(files[j], "r");
                if (file == NULL) continue;
                char line[1000];
                while (fgets(line, sizeof(line), file)) {
                    if (strstr(line, keyword) != NULL) {
                        char message[MAX_FILENAME + 50];
                        sprintf(message, "子进程'%d'找到了文件：%s", pid, files[j]);
                        write(pipefd[1], message, strlen(message) + 1);
                        break;
                    }
                }
                fclose(file);
            }
            close(pipefd[1]);  // 关闭管道的写端
            exit(0);
        }
    }

    // 父进程
    close(pipefd[1]);  // 关闭管道的写端
    char filename[MAX_FILENAME];
    while (read(pipefd[0], filename, MAX_FILENAME) > 0) {
        printf("%s\n", filename);
    }

    for (int i = 0; i < NUM_CHILDREN; i++) {
        wait(NULL);
    }

    return 0;
}