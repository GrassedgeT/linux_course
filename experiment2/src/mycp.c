#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    int srcFile, destFile;
    char *buffer[BUF_SIZE];
    ssize_t bytesRead, bytesWritten, pos;

    if (argc != 3) {
        write(2, "所需参数：原文件名 目标文件名\n", 31);
        exit(1);
    }

    srcFile = open(argv[1], O_RDONLY);
    if (srcFile == -1) {
        perror("open");
        exit(1);
    }

    destFile = open(argv[2], O_WRONLY | O_CREAT, 0644);
    if (destFile == -1) {
        perror("open");
        exit(1);
    }

    while ((bytesRead = read(srcFile, buffer, BUF_SIZE)) > 0) {
        pos = 0;
        do {
            bytesWritten = write(destFile, buffer + pos, bytesRead - pos);
            if (bytesWritten >= 0) {
                pos += bytesWritten;
            } else if (bytesWritten == -1) {
                perror("write");
                exit(1);
            }
        } while (bytesRead > pos);
    }

    if (bytesRead == -1) {
        perror("read");
        exit(1);
    }

    if (close(srcFile) == -1 || close(destFile) == -1) {
        perror("close");
        exit(1);
    }

    return 0;
}