#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
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

int main() {
    char* cost_file = "../execution_time/dryer.in";
    char* table_file = "table";

    FILE* table = fopen(table_file, "r");
    if (table == NULL) {
        perror("fopen");
        return ERR_MEM;
    }
    
    key_t k = ftok("pathname", 1);
    if (k < 0) {
        perror("ftok");
        fprintf(table, "%s\n", "stop");
        return ERR_MEM;
    }
    int sem_queue = semget(k, 1, IPC_CREAT | 0660);
    if (sem_queue < 0) {
        perror("semget");
        return ERR_MEM;
    }
    struct sembuf take = {0, 1, 0};    
    char type[4096];
    int num = 0;
    while(1) {
        while(fscanf(table, "%s", type) != EOF) {
            if (strcmp(type, "stop") == 0) {
                printf("finish drying\n");
                fclose(table);
                return ERR_NONE;
            }
            int time = working_time(cost_file, type);
            if (time < 0)
                continue;
            if (semop(sem_queue, &take, 1) < 0) {
                perror("semop");
                return ERR_MEM;
            }
            ++num;
            printf("taken %s\n", type);
            sleep(time);
            printf("dried %s\n", type);
        }
        fclose(table);
        table = fopen(table_file, "r");
        for (int i = 0; i < num; ++i) {
            fscanf(table, "%s", type);
        }
    }
    printf("finish drying\n");
    fclose(table);
    return ERR_NONE;
}