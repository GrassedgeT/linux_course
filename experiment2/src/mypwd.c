#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>

#define MAX_DEPTH 1024

void print_path(ino_t cur_ino, char path[MAX_DEPTH][NAME_MAX+1], int *depth) {
    DIR *dir = opendir("..");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_ino == cur_ino) {
            strncpy(path[*depth], entry->d_name, NAME_MAX);
            (*depth)++;
            break;
        }
    }

    closedir(dir);
}

int main() {
    struct stat st;
    if (stat(".", &st) == -1) {
        perror("stat");
        return 1;
    }

    char path[MAX_DEPTH][NAME_MAX+1];
    int depth = 0;

    ino_t cur_ino = st.st_ino;
    while (cur_ino != 2) {  // 2 is the inode number of root directory
        print_path(cur_ino, path, &depth);
        if (chdir("..") == -1) {
            perror("chdir");
            return 1;
        }
        if (stat(".", &st) == -1) {
            perror("stat");
            return 1;
        }
        cur_ino = st.st_ino;
    }

    for (int i = depth - 1; i >= 0; i--) {
        printf("/%s", path[i]);
    }
    printf("\n");

    return 0;
}