#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    
    if (argc != 2){
        printf("uncorrect number of arguments\n");
        return -1;
    }
    FILE* file;
    
    char cmd_line[4096];
    char sep[] = " ";
    int num, counter = 0;
    if ((file =  fopen(argv[1], "r")) == NULL){
        perror("fopen");
        return -2;
    }
    
    while(fscanf(file, "%d %[^\n]%*c", &num, &cmd_line) != EOF) {
        
        int i = 0;
        char* argl[2048]  = {};
        char* istr;
        istr = strtok(cmd_line, sep);
        while(istr != NULL) {
            argl[i++] = istr;
            istr = strtok(NULL, sep);
        }
        argl[i] = NULL;
        ++counter;
        pid_t p = fork();
        if (p == 0) {   //child process
            sleep(num);
            int ret = execvp(argl[0], argl);
            if (ret == -1) {
                perror("execvp");
                return -3;
            }   
        }
        else if (p == -1) {
            perror("fork");
            return -4;
        }
    }
    int status;
    for (int i = 0; i < counter; ++i)
        wait(&status);
    fclose(file);
}