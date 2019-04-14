#include <time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>

void get_rusage_string(char *buffer, size_t buffer_len) { 
    struct rusage *usage = malloc(sizeof(struct rusage));
    getrusage(RUSAGE_SELF, usage);
    snprintf(buffer, buffer_len, "utime: %ld\nstime: %ld\nmaxrss: %ld\nixrss: %ld\nidrss: %ld\nisrss: %ld\nminflt: %ld\nmajflt: %ld\nnswap: %ld\ninblock: %ld\noublock: %ld\nmsgsnd: %ld\nmsgrcv: %ld\nnsignals: %ld\nnvcsw: %ld\nnivcsw: %ld\n", 
            usage->ru_utime.tv_usec, usage->ru_stime.tv_usec, usage->ru_maxrss, usage->ru_ixrss,
            usage->ru_idrss, usage->ru_isrss, usage->ru_minflt, usage->ru_majflt, usage->ru_nswap, usage->ru_inblock, 
            usage->ru_oublock, usage->ru_msgsnd, usage->ru_msgrcv, usage->ru_nsignals, usage->ru_nvcsw, usage->ru_nivcsw);
    free(usage);
}

void get_command_rusage(int argc, char * const argv[], char *buffer, size_t buffer_len){
    pid_t child_pid = fork();
    if(child_pid == 0){
        execvp(argv[0], argv);
    } else if(child_pid > 0) {
        struct rusage *usage = malloc(sizeof(struct rusage));
        wait4(child_pid, NULL, NULL, usage);
        snprintf(buffer, buffer_len, "utime: %ld\nstime: %ld\nmaxrss: %ld\nixrss: %ld\nidrss: %ld\nisrss: %ld\nminflt: %ld\nmajflt: %ld\nnswap: %ld\ninblock: %ld\noublock: %ld\nmsgsnd: %ld\nmsgrcv: %ld\nnsignals: %ld\nnvcsw: %ld\nnivcsw: %ld\n", 
                usage->ru_utime.tv_usec, usage->ru_stime.tv_usec, usage->ru_maxrss, usage->ru_ixrss,
                usage->ru_idrss, usage->ru_isrss, usage->ru_minflt, usage->ru_majflt, usage->ru_nswap, usage->ru_inblock, 
                usage->ru_oublock, usage->ru_msgsnd, usage->ru_msgrcv, usage->ru_nsignals, usage->ru_nvcsw, usage->ru_nivcsw);
        free(usage);
    }
}