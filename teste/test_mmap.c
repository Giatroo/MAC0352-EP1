#include <stdio.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    int N = 5; // Number of elements for the array

    int **ptr = mmap(NULL, N * sizeof(int *), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    int *ptr1 = mmap(NULL, N * sizeof(int), PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    if (ptr == MAP_FAILED) {
        printf("Mapping Failed\n");
        return 1;
    }

    for (int i = 0; i < N; i++)
        ptr[i] = mmap(NULL, 1 * sizeof(int), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    fprintf(stderr, "Initial values of the array elements :\n");
    for (int i = 0; i < N; i++) { fprintf(stderr, " %d", ptr[i][0]); }
    fprintf(stderr, "\n");

    pid_t child_pid = fork();

    if (child_pid == 0) {
        // child 1
        for (int i = 0; i < N; i++) { ptr[i][0] = i*i*i + 1; }
        fprintf(stderr, "Child1:\n");
        fprintf(stderr, "Initial values of the array elements :\n");
        for (int i = 0; i < N; i++) { fprintf(stderr, " cima %d", ptr[i][0]); }
        fprintf(stderr, "\n");

        return 0;
    } else {
        pid_t child2_pid = fork();

        if (child2_pid == 0) {
            waitpid(child_pid, NULL, 0);
            fprintf(stderr, "\nChild2:\n");

            fprintf(stderr, "Updated values of the array elements 1:\n");
            for (int i = 0; i < N; i++) {
                fprintf(stderr, " baixo %d", ptr[i][0]);
            }
            fprintf(stderr, "\n");
        } else {
            fprintf(stdout, "\nParent\n");
            // parent
            waitpid(child_pid, NULL, 0);
            waitpid(child2_pid, NULL, 0);
        }
    }

    printf("Passei aqui \n");

    /* int err = munmap(ptr, N * sizeof(int)); */

    /* if (err != 0) { */
        /* printf("UnMapping Failed\n"); */
        /* return 1; */
    /* } */
    return 0;
}
