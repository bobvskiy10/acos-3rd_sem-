#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


int main() {
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