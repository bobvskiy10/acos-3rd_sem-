#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
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
    char* cost_file = "../execution_time/washer.in";
    char* queue_file = "../queue.in";
    char* table_file = "table";
    int TABLE_LIMIT = table_limit();
    if (TABLE_LIMIT < 0) {
        printf("uncorrect table limit\n");
        return ERR_IO;
    }
    FILE* table = fopen(table_file, "w");
    if (table == NULL) {
        perror("fopen");
        return ERR_MEM;
    }
    FILE* queue = fopen(queue_file, "r");
    if (queue == NULL) {
        perror("fopen");
        fprintf(table, "%s\n", "stop");
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
        fprintf(table, "%s\n", "stop");
        return ERR_MEM;
    }
    struct sembuf fill = {0, TABLE_LIMIT, 0};
    struct sembuf unfill = {0, -TABLE_LIMIT, 0};
    struct sembuf put = {0, -1, 0};
    if (semop(sem_queue, &fill, 1) < 0){
        perror("semop");
        fprintf(table, "%s\n", "stop");
        return ERR_MEM;
    }

    
    char type[4096];
    int number;
    while(fscanf(queue, "%s : %d", type, &number) != EOF) {
        int time = working_time(cost_file, type);
        if (time < 0)
            continue;
        for (int i = 0; i < number; ++i) {
            sleep(time);
            if (semop(sem_queue, &put, 1) < 0){
                perror("semop");
                fprintf(table, "%s\n", "stop");
                fflush(table);
                return ERR_MEM;
            }
            printf("washed %s\n", type);
            fprintf(table, "%s\n", type);
            fflush(table);
        }
    }
    printf("finish washing\n"); 
    fprintf(table, "%s\n", "stop");
    fflush(table);
    fclose(table);
    fclose(queue);
    if (semop(sem_queue, &unfill, 1) < 0){
        perror("semop");
        fprintf(table, "%s\n", "stop");
        return ERR_MEM;
    }
    int status;
    wait(&status);
    return ERR_NONE;
}