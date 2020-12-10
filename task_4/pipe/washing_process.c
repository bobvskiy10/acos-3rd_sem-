#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

enum {
    ERR_NONE = 0,
    ERR_IO,
    ERR_MEM
};

int working_time(char* file_name, char* type) {
    int time;
    char buf[128];
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        perror("fopen");
        return -1;
    }
    while(fscanf(file, "%s : %d", buf, &time) != EOF) {
        if (strcmp(type, buf) == 0) {
            fclose(file);
            return time;
        }
    }
    printf("Unknow type %s\n", type);
    fclose(file);
    return -1;
}

int table_limit() {
    char* table_limit_string = getenv("TABLE_LIMIT");
    if (table_limit_string == NULL)
        return -1; 
    char* end;
    int lim;
    lim = strtol(table_limit_string, &end, 0);
    if (end == table_limit_string){
        printf("not number\n");
        return -1;
    }
    return lim;
}

int main() {
    char* washing_cost = "../execution_time/washer.in";
    char* drying_cost = "../execution_time/dryer.in";
    char* queue_file = "../queue.in";
    int TABLE_LIMIT = table_limit();
    if (TABLE_LIMIT < 0) {
        printf("uncorrect table limit\n");
        return ERR_IO;
    }


    int fqueue[2];
    if (pipe(fqueue) == -1) {
        perror("pipe");
        return ERR_MEM;
    }

    int ftable[2];
    if (pipe(ftable) == -1) {
        perror("pipe");
        return ERR_MEM;
    }

    pid_t p = fork();

    if (p > 0) {
        close(fqueue[0]);
        close(ftable[1]);
        FILE* queue = fopen(queue_file, "r");
        if (queue == NULL) {
            perror("fopen");
            close(fqueue[0]);
            close(ftable[1]);
            return ERR_MEM;
        }

        int counter = 0;
        char type[4096];
        int number;
        while(fscanf(queue, "%s : %d", type, &number) != EOF) {
            int time = working_time(washing_cost, type);
            int type_size = strlen(type);
            type[type_size] = ' ';
            if (time < 0)
                continue;
            for (int i = 0; i < number; ++i) {
                sleep(time);
                if (counter >= TABLE_LIMIT) {
                    char buf[2];
                    int nb = 0;
                    nb = read(ftable[0], buf, 1);
                    if (nb == 0) {
                        printf("dryer gone away\n");
                        return ERR_MEM;
                    }
                    --counter;
                }
                ++counter;
                printf("washed %s\n", type);
                if (write(fqueue[1], type, type_size + 1) < type_size + 1) {
                    perror("write");
                    return ERR_MEM;
                }
            }
        }
        char stop_signal[] = "stop";
        if(write(fqueue[1], stop_signal, sizeof(stop_signal) - 1) < sizeof(stop_signal) - 1) {
            perror("write");
            return ERR_MEM;
        }
        printf("finish washing\n"); 
        int status;
        wait(&status);
        close(fqueue[1]);
        close(ftable[0]);  
        
    }

    else if (p == 0) {
        close(fqueue[1]);
        close(ftable[0]);

        char buf[4096 + 1];
        int nb = 0;
        char sep[2] = " ";
        while((nb = read(fqueue[0], buf, 4096)) > 0) {
            buf[nb] = '\0';

            char* istr;
            char com[1024];
            istr = strtok(buf, sep);
            while(istr != NULL) {
                strcpy(com, istr);
                if (strcmp(com, "stop") == 0) {
                    printf("finish drying\n");
                    close(fqueue[0]);
                    close(ftable[1]); 
                    return ERR_NONE;
                }
                int time = working_time(drying_cost, istr);
                istr = strtok(NULL, sep);
                if (write(ftable[1], "1", 1) < 1) {
                    perror("write");
                    return ERR_MEM;
                }
                if (time < 0)
                    continue;
                printf("taken %s\n", com);
                sleep(time);
                printf("dried %s\n", com);
                
            }
        }
        printf("finish drying\n");
        close(fqueue[0]);
        close(ftable[1]); 
        return ERR_NONE;
    }


    else if (p == -1) {
        perror("fork");
        close(fqueue[1]);
        close(ftable[0]);
        close(fqueue[0]);
        close(ftable[1]); 
        return ERR_MEM;
    }
    return ERR_NONE;
}