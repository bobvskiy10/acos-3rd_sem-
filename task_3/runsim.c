#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

int n = 0;  /*processes counter*/

void child_sig(int sig) {
    pid_t p;
    int status;
    while((p = waitpid(-1, &status, WNOHANG)) > 0) {
        --n;
    }
}

enum {
    ERR_NONE = 0,
    ERR_IO,
    ERR_MEM
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("uncorrect number of arguments\n");
        return ERR_IO;
    }
    signal(SIGCHLD, child_sig);
    char show_number[] = "show number of processes";  /*command to show how many processe executing now*/
    char* end;
    int num;    /*maximum of processes*/
    num = strtol(argv[1], &end, 0);
    if (end == argv[1]) {
        printf("not number\n");
        return ERR_IO;
    }
    if (num <= 0) {
        printf("uncorect N\n");
        return ERR_IO;
    }
    char cmd_line[4096];
    char sep[] = " ";  /*array of separators for strtok*/
    
    while(scanf("%[^\n]%*c", &cmd_line) != EOF) {
        if (strcmp(show_number, cmd_line) == 0) {
            printf("now executing %d/%d processes\n", n, num);
            continue;
        }
        if (n >= num) {
            printf("Too many process\n");
            continue;
        }
        int i = 0;
        char* argl[2048]  = {};
        char* istr;
        istr = strtok(cmd_line, sep);
        while(istr != NULL) {
            argl[i++] = istr;
            istr = strtok(NULL, sep);
        }
        argl[i] = NULL;
        pid_t p = fork();
        if (p == 0) {
            execvp(argl[0], argl);
            perror("execvp");
            return ERR_MEM;
        }
        else if (p == -1) {
            perror("fork");
            return ERR_MEM;
        }
        ++n;
    }

    return ERR_NONE;
}