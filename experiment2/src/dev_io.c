#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    char *tty = ttyname(STDIN_FILENO);
    if (tty == NULL) {
        perror("ttyname");
        return 1;
    }

    int fd = open(tty, O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    char *msg = "Program write msg: Hello, world!\n";
    if (write(fd, msg, strlen(msg)) == -1) {
        perror("write");
        return 1;
    }

    char buf[1024];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n == -1) {
        perror("read");
        return 1;
    }

    buf[n] = '\0';
    printf("Read: %s", buf);

    close(fd);
    return 0;
}