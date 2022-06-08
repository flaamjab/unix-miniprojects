#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "matrix.h"

#define FIFO "/tmp/25b4e327-47b3-476f-8900-b8dd562169ed-%d.fifo"

void fifo_pipe_path(int id, char** pipe) {
    int n = snprintf(NULL, 0, FIFO, 5);
    free(*pipe);
    *pipe = malloc(n);
    sprintf(*pipe, FIFO, id);
}

void fifo_pipe_new(const char* path) {
    umask(0);
    mknod(path, S_IFIFO|0666, 0);
}

void split_jobs(int c_jobs, int c_workers, int* workers) {
    int jobs_per_worker = c_jobs / c_workers;
    int leftover_jobs = c_jobs % c_workers;
    for (int w = 0; w < c_workers; w++) {
        workers[w] = jobs_per_worker;
        if (leftover_jobs > 0) {
            workers[w]++;
            leftover_jobs--;
        }
    }
}

int main() {
    FILE* matrix_file = fopen("matrix.txt", "r");
    if (matrix_file == NULL) {
        perror("matrix.txt");
        exit(EXIT_FAILURE);
    }

    FILE* vector_file = fopen("vector.txt", "r");
    if (vector_file == NULL) {
        perror("vector.txt");
        exit(EXIT_FAILURE);
    }

    Matrix* m = matrix_from_file(matrix_file);
    Matrix* v = matrix_from_file(vector_file);

    fclose(matrix_file);
    fclose(vector_file);

    int c_workers = 3;
    int workers[c_workers];
    size_t c_jobs = matrix_rows_num(m);
    split_jobs(c_jobs, c_workers, workers);
    
    char* pipe_path = NULL;
    int first_row = 0;
    int p_id = 0;
    for (int w = 0; w < c_workers; w++) {
        p_id = fork();
        fifo_pipe_path(w, &pipe_path);
        fifo_pipe_new(pipe_path);

        if (p_id == 0) {
            Matrix* mr = matrix_rows(m, first_row, first_row + workers[w]);

            // Multiply matrix by vector
            Matrix* r = matrix_mult(mr, v);
            printf(
                "Child #%d, from %d to %d, result:\n",
                w, first_row, first_row + workers[w]
            );
            matrix_print(r);

            // Write result
            FILE* pipe = fopen(pipe_path, "w");
            matrix_write_to_file(r, pipe);
            fclose(pipe);

            printf("Child #%d is done\n", w);
            break;
        } else {
            first_row += workers[w];
        }
    }

    if (p_id != 0) {
        FILE* output = fopen("result.txt", "w");
        for (int w = 0; w < c_workers; w++) {
            fifo_pipe_path(w, &pipe_path);

            FILE* pipe = fopen(pipe_path, "r");

            char buf[32];
            while (!feof(pipe)) {
                int n = fread(buf, sizeof(char), 32, pipe);
                if (n > 0) {
                    fwrite(buf, sizeof(char), n, output);
                }
            }

            fclose(pipe);
        }
        fclose(output);
    }

    free(pipe_path);
}
