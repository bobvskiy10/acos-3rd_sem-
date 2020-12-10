#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int main() {
    int fd = open("table", O_WRONLY | O_CREAT | O_TRUNC, 0660);
    if (fd == -1) {
        perror("open");
        return -1;
    }
    close(fd);
    pid_t p = fork();
    if (p > 0) { 
        execl("./washer", "./washer", NULL);
        perror("execl");
        return 1;
    }
    else if (p == 0) {
        execl("./dryer", "./dryer", NULL);
        perror("execl");
        return 1;
    }
    else {
        perror("fork");
        return 2;
    }
}