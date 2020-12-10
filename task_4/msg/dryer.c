#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

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

struct MSG1 {
        long mtype;
        char type[4096];
};

struct MSG2 {
     long mtype;
};

int main() {
    char* cost_file = "../execution_time/dryer.in";
    char* queue_file = "../queue.in";

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

    struct MSG1 rcv;
    struct MSG2 snd;
    snd.mtype = 2;
    while(1){
        if (msgrcv(id, &rcv, sizeof(rcv) - sizeof(long), 1, 0) < 0) {
            perror("msgrcv");
            return ERR_MEM;
        }
        if (strcmp("stop", rcv.type) == 0) {
            printf("finish drying\n");
             break;
        }
        if (msgsnd(id, &snd, sizeof(snd) - sizeof(long), 0) < 0) {
                perror("msgsnd");
                return ERR_MEM;
            }
        int time = working_time(cost_file, rcv.type);
        if (time < 0)
            continue;
            printf("taken %s\n", rcv.type);
        sleep(time);
        printf("dryed %s\n", rcv.type);
    }
    return ERR_NONE;
}