#include "assign5.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <aio.h>
#include <errno.h>

size_t copyFiles(const int inputs[], const int outputs[], size_t length, size_t bufferSize, bool async, size_t *time) {
    struct timespec start, end;
    int totalBytes;
    if (!async) {
        printf("Copying sync files...\n");
        for (int i = 0; i < length; i++) {
            struct stat statbuf;
            if (fstat(inputs[i], &statbuf) == -1) {
                printf("Error reading file stats. Exiting.\n");
                exit(0);
            }
            void *buf = malloc(statbuf.st_size);
            if (lseek(inputs[i], 0, SEEK_SET) == -1) {
                printf("Error seeking file. Exiting.\n");
                exit(0);
            }
            clock_gettime(CLOCK_REALTIME, &start);
            if (read(inputs[i], buf, statbuf.st_size) == -1) {
                printf("Error reading file. Exiting.\n");
                exit(0);
            }
            if (write(outputs[i], buf, statbuf.st_size) == -1) {
                printf("Error writing file. Exiting.\n");
                exit(0);
            }
            clock_gettime(CLOCK_REALTIME, &end);
            *time += (end.tv_nsec - start.tv_nsec)/1000;
            totalBytes += statbuf.st_size;
            free(buf);
        }
    }
    else {
        printf("Copying async files...\n");
        struct aiocb **reqs = malloc(sizeof(struct aiocb*)*length);
        for (int i = 0; i < length; i++) {
            struct stat statbuf;
            if (fstat(inputs[i], &statbuf) == -1) {
                printf("Error reading file stats. Exiting.\n");
                exit(0);
            }
            void *buf = malloc(statbuf.st_size);
            struct aiocb* aios = malloc(sizeof(struct aiocb));
            aios->aio_fildes = inputs[i];
            aios->aio_buf = buf;
            aios->aio_nbytes = statbuf.st_size;
            aios->aio_offset = 0;
            clock_gettime(CLOCK_REALTIME, &start);
            if (aio_read(aios) == -1) {
                printf("Error async reading file (%d). Exiting.\n", errno);
                exit(0);
            }
            totalBytes += statbuf.st_size;
            free(buf);
            reqs[i] = aios;
        }
        int openReqs = length;
        while (openReqs > 0) {
            int currentReqs = 0;
            for (int i = 0; i < length; i++) {
                if (aio_error(reqs[i]) == EINPROGRESS) {
                    currentReqs++;
                }
            }
            openReqs = currentReqs;
        }
        clock_gettime(CLOCK_REALTIME, &end);
        *time += (end.tv_nsec - start.tv_nsec)/1000;
        for (int i = 0; i < length; i++) {
            reqs[i]->aio_fildes = outputs[i];
            reqs[i]->aio_reqprio = 1;
            clock_gettime(CLOCK_REALTIME, &start);
            if (aio_write(reqs[i]) == -1) {
                printf("Error async writing file (%d). Exiting.\n", errno);
                exit(0);
            }
        }
        openReqs = length;
        while (openReqs > 0) {
            int currentReqs = 0;
            for (int i = 0; i < length; i++) {
                if (aio_error(reqs[i]) == EINPROGRESS) {
                    currentReqs++;
                }
            }
            openReqs = currentReqs;
        }
        clock_gettime(CLOCK_REALTIME, &end);
        *time += (end.tv_nsec - start.tv_nsec)/1000;
        for (int i = 0; i < length; i++) {
            free(reqs[i]);
        }
        free(reqs);
    }
    return totalBytes;
}
