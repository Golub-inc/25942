#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void) {
pid_t pid = fork();
if (pid == 0) {
    execlp("cat", "cat", "long_test.txt", (char *)NULL);
}
else {
    printf("i am very cool parent process\n-----only-stupid-children-below-----\n");
    wait(NULL);
    printf("\n-----only-cool-parents-below-----\nthat's all folks!\n");
}
    return 0;
}