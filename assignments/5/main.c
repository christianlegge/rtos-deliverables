#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include "assign5.h"

int main(int argc, char *argv[]) {
    int *inputs = malloc(sizeof(int)*(argc-1));
    int *syncoutputs = malloc(sizeof(int)*(argc-1));
    int *asyncoutputs = malloc(sizeof(int)*(argc-1));


    for (int i = 0; i < argc - 1; i++) {
        inputs[i] = open(argv[i+1], O_RDONLY);
        if (inputs[i] == -1) {
            printf("Unable to open file %s. Exiting.\n", argv[i+1]);
            return 1;
        }
        syncoutputs[i] = open(strcat(strdup(argv[i+1]), ".sync"), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (syncoutputs[i] == -1) {
            printf("Unable to open file %s. Exiting.\n", strcat(strdup(argv[i+1]), ".sync"));
            return 1;
        }
        asyncoutputs[i] = open(strcat(strdup(argv[i+1]), ".async"), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (asyncoutputs[i] == -1) {
            printf("Unable to open file %s. Exiting.\n", strcat(strdup(argv[i+1]), ".async"));
            return 1;
        }
    }

    size_t synctime = 0;
    size_t asynctime = 0;
    copyFiles(inputs, syncoutputs, argc-1, 0, 0, &synctime);
    copyFiles(inputs, asyncoutputs, argc-1, 0, 1, &asynctime);
    printf("Sync time: \t%d us\nAsync time: \t%d us\n", synctime, asynctime);
}
