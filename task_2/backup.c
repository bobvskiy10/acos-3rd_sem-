#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <zlib.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

void archivate(char* from, char* to) {
    int from_dir;

    if ((from_dir = open(from, O_RDONLY, 0)) == -1) {
        perror("open");
        printf("can't open and archivate %s\n", from);
        return;
    }
    char buf[4096];
    gzFile to_dir = gzopen(to, "w");
    if (!to_dir) {
        perror("gzopen");
        printf("can't open %s\n", to);
        return;
    }
    int len = -1;
    while ((len = read(from_dir, buf, sizeof(buf))) > 0) {
        if(gzwrite(to_dir, buf, len) <= 0) {
            perror("gzwrite");
            printf("can't write to %s\n", to);
            close(from_dir);
            return;
        }
    }
    gzclose(to_dir);
    close(from_dir);
    return;
}


void copy(char* from, char* to) {
    DIR* cur = opendir(from);
    if (cur == NULL) {
        perror("opendir");
        printf("can't open %s\n", from);
        return;
    }

    struct dirent* from_d = NULL;
    struct stat from_info;
    struct stat to_info;
    char from_name[256] = "", to_name[256] = "";
    while((from_d = readdir(cur)) != NULL) {
        if (strcmp(from_d->d_name, ".") == 0 || strcmp(from_d->d_name, "..") == 0)
            continue;
        
        sprintf(from_name, "%s/%s", from, from_d->d_name);
        sprintf(to_name, "%s/%s", to, from_d->d_name);

        stat(from_name, &from_info);

        if (S_ISDIR(from_info.st_mode)) {
            if (stat(to_name, &to_info) == -1)
                mkdir(to_name, 0775);
            copy(from_name, to_name);
            continue;
        }

        if (S_ISREG(from_info.st_mode)) {
            sprintf(to_name, "%s.gz", to_name);
            if (stat(to_name, &to_info) == 0) {
                if (to_info.st_mtime < from_info.st_mtime) 
                    unlink(to_name);
                else 
                    continue;

            }

            archivate(from_name, to_name);
        }
    }
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("uncorrect number of arguments\n");
        return -1;
    }

    copy(argv[1], argv[2]);
    return 0;

}