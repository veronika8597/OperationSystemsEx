#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    while (1) {
        char buf[1000];
        printf(">>>");
        fflush(stdout);
        if (fgets(buf, 1000, stdin) == NULL) {
            if (feof(stdin)) {
                // end-of-file, exit program
                printf("\n");
                exit(EXIT_SUCCESS);
            } else {
                // I/O error, exit program with error
                perror("fgets");
                exit(EXIT_FAILURE);
            }
        }
        buf[strcspn(buf, "\n")] = '\0'; // remove newline character
        if (strcmp(buf, "leave") == 0)
            break;

        pid_t pid = fork();
        if (pid == -1) {
            // fork error, exit program with error
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // child process
            char command[2000];
            char *argv[3] = { NULL };
            char *path = getenv("PATH");
            char *dir = strtok(path, ":");
            while (dir != NULL) {
                sprintf(command, "%s/%s", dir, buf);
                if (access(command, X_OK) == 0) {
                    argv[0] = command;
                    if (execv(command, argv) == -1) {
                        perror("execv");
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
                dir = strtok(NULL, ":");
            }
            if (dir == NULL) {
                fprintf(stderr, "Command not found: %s\n", buf);
                exit(EXIT_FAILURE);
            }
        } else {
            // parent process
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
            if (WIFEXITED(status)) {
                printf("Child process exited with status %d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child process terminated by signal %d\n", WTERMSIG(status));
            }
        }
    }
    return 0;
}
