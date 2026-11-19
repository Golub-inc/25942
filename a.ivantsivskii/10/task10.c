#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    pid_t pid;
    int status;
    if (argc < 2) {
        return 1;
    }
    pid = fork();
    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        _exit(127);
    }
    if (waitpid(pid, &status, 0) == -1) {
        return 1;
    }
    if (WIFEXITED(status)) {
        printf("%d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("signal %d\n", WTERMSIG(status));
    }
    return 0;
}