#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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

struct MSG1 {
        long mtype;
        char type[4096];
};

struct MSG2 {
     long mtype;
};

int main() {
    char* cost_file = "../execution_time/washer.in";
    char* queue_file = "../queue.in";

    int TABLE_LIMIT = table_limit();
    if (TABLE_LIMIT < 0) {
        printf("uncorrect table limit\n");
        return ERR_IO;
    }
    FILE* queue = fopen(queue_file, "r");
    if (queue == NULL) {
        perror("fopen");
        return ERR_MEM;
    }
    key_t k = ftok("pathname", 1);
    if (k < 0) {
        perror("ftok");
        return ERR_MEM;
    }

    int id = msgget(k, IPC_CREAT | 0660);
    if (id < 0 ){
        perror("msgget");
        return ERR_MEM;
    }

    struct MSG1 snd;
    snd.mtype = 1;
    struct MSG2 rcv;
    char type[4096];
    int number, counter = 0;
    while(fscanf(queue, "%s : %d", type, &number) != EOF) {
        int time = working_time(cost_file, type);
        if (time < 0)
            continue;
        for (int i = 0; i < number; ++i) {
            sleep(time);
            if (counter >= TABLE_LIMIT) {
                if (msgrcv(id, &rcv, sizeof(rcv) - sizeof(long), 2, 0) < 0) {
                    perror("msgrcv");
                    return ERR_MEM;
                }
                --counter;
            }   
            ++counter;
            printf("washed %s\n", type);    
            strcpy(snd.type, type);
            if (msgsnd(id, &snd, sizeof(snd) - sizeof(long), 0) < 0) {
                perror("msgsnd");
                return ERR_MEM;
            }
        }
    }
    strcpy(snd.type, "stop");
    if (msgsnd(id, &snd, sizeof(snd) - sizeof(long), 0) < 0) {
        perror("msgsnd");
        return ERR_MEM;
    }
    for (int i = 0; i < TABLE_LIMIT; ++i) {
        if (msgrcv(id, &rcv, sizeof(rcv) - sizeof(long), 2, 0) < 0) {
            perror("msgrcv");
            return ERR_MEM;
        }
    }

    int status;
    wait(&status);
    return ERR_NONE;
}
