#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t p = fork();
    if (p == -1) {
        perror("fork");
            return -1;
    }
    if (p > 0) { 
        pid_t q = fork();
        if (q == -1) {
            perror("fork");
                return -1;
        }
        else if (q == 0) {
            int ret = execl("./integral.exe", "./integral.exe", "0.0", "1.4", "0.001", NULL);
            if (ret == -1) {
                perror("execl");
                return -1;
            }
        }
    }
    else if (p == 0){
        int ret = execl("./integral.exe", "./integral.exe", "-0.9", "4.71", "0.01", NULL);
        if (ret == -1) {
            perror("execl");
            return -1;
        }
            
    }
    return 0;
}