#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/sem.h>

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

struct Type{
    char type[256];
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

    int shmid = shmget(k, TABLE_LIMIT * sizeof(struct Type), IPC_CREAT | 0666);

    if (shmid < 0) {
        /*if (errno != EEXIST) {
            printf("iugsfiuges");
            fflush(stdout);
            perror("shmget");
            return ERR_MEM;
        }
        if (shmid = shmget(k, TABLE_LIMIT * sizeof(struct Type), 0) < 0) {
            perror("shmget");
            return ERR_MEM;
        }*/
        perror("shmget");
        return ERR_MEM;
    }

    struct Type* mem = (struct Type*)shmat(shmid, NULL, 0);
    if (mem == (struct Type*)(-1)) {
        perror("schmat");
        return ERR_MEM;
    }

    key_t k_s1 = ftok("pathname", 2);
    if (k_s1 < 0) {
        perror("ftok");
        return ERR_MEM;
    }
    int sem_table = semget(k_s1, 1, IPC_CREAT | 0660);
    if (sem_table < 0 ) {
        perror("semget");
        return ERR_MEM;
    }
    struct sembuf fill = {0, TABLE_LIMIT, 0};
    struct sembuf unfill = {0, -TABLE_LIMIT, 0};
    struct sembuf put = {0, -1, 0};
    struct sembuf push = {0, 1, 0};

    if (semop(sem_table, &fill, 1) < 0){
        perror("semop");
        return ERR_MEM;
    }

    key_t k_s2 = ftok("pathname", 3);
    if (k_s2 < 0) {
        perror("ftok");
        return ERR_MEM;
    }

    int sem_is_on_table = semget(k_s2, 1, IPC_CREAT | 0660);
    if (sem_is_on_table < 0 ) {
        perror("semget");
        return ERR_MEM;
    }
    char type[4096];
    int number, pos =0;
    while(fscanf(queue, "%s : %d", type, &number) != EOF) {
        int time = working_time(cost_file, type);
        if (time < 0)
            continue;
        for (int i = 0; i < number; ++i) {
            sleep(time);
            if (semop(sem_table, &put, 1) < 0){
                perror("semop");
                strcpy(mem[pos].type, "stop");
                return ERR_MEM;
            }

            if (semop(sem_is_on_table, &push, 1) < 0) {
                perror("semop");
                strcpy(mem[pos].type, "stop");
                return ERR_MEM;
            }
            strcpy(mem[pos].type, type);
            pos = (pos + 1) % TABLE_LIMIT;
            printf("washed %s\n", type);
        }
    }
    printf("finish washing\n"); 
    if (semop(sem_table, &put, 1) < 0){
            perror("semop");
            strcpy(mem[pos].type, "stop");
            return ERR_MEM;
    }
    if (semop(sem_is_on_table, &push, 1) < 0) {
        perror("semop");
        strcpy(mem[pos].type, "stop");
        return ERR_MEM;
    }
    fclose(queue);
    strcpy(mem[pos].type, "stop");
    if (semop(sem_is_on_table, &push, 1) < 0) {
        perror("semop");
        strcpy(mem[pos].type, "stop");
        return ERR_MEM;
    }
    int status;
    wait(&status);
    fflush(stdout);
    struct sembuf clear = {0, -1, IPC_NOWAIT};
    while(semop(sem_table, &clear, 1) >= 0) {};
    while(semop(sem_is_on_table, &clear, 1) >= 0) {};
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl");
        return ERR_MEM;
    }
    return ERR_NONE;
}