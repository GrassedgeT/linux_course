#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <linux/limits.h>

void copy_file(const char *src, const char *dst) {
    int src_fd = open(src, O_RDONLY);
    int dst_fd = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (src_fd == -1 || dst_fd == -1) {
        perror("open");
        return;
    }

    char buf[4096];
    ssize_t n;
    while ((n = read(src_fd, buf, sizeof(buf))) > 0) {
        if (write(dst_fd, buf, n) != n) {
            perror("write");
            return;
        }
    }

    if (n == -1) {
        perror("read");
    }

    close(src_fd);
    close(dst_fd);
}

void copy_dir(const char *src, const char *dst) {
    DIR *dir = opendir(src);
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    if (mkdir(dst, 0755) == -1) {
        perror("mkdir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char src_path[PATH_MAX];
        snprintf(src_path, sizeof(src_path), "%s/%s", src, entry->d_name);

        char dst_path[PATH_MAX];
        snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, entry->d_name);

        struct stat st;
        if (stat(src_path, &st) == -1) {
            perror("stat");
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            copy_dir(src_path, dst_path);
        } else if (S_ISREG(st.st_mode)) {
            copy_file(src_path, dst_path);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return 1;
    }

    copy_dir(argv[1], argv[2]);
    return 0;
}