#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "matrix.h"

typedef struct _Matrix {
    double* numbers;
    size_t rows;
    size_t cols;
} Matrix;

Matrix* matrix_new(
    double* numbers,
    size_t size,
    size_t rows,
    size_t cols
) {
    if (rows * cols != size) {
        fprintf(
            stderr,
            "The matrix dimensions do not match (%ld * %ld != %ld)",
            rows, cols, size
        );
        exit(EXIT_FAILURE);
    }

    Matrix* m = (Matrix*)malloc(sizeof(Matrix));
    m->rows = rows;
    m->cols = cols;
    m->numbers = numbers;

    return m;
}

Matrix* matrix_zeroes(size_t rows, size_t cols) {
    int size = rows * cols;
    double* numbers = malloc(sizeof(double) * size);
    return matrix_new(numbers, size, rows, cols);
}

Matrix* matrix_empty() {
    return matrix_new(NULL, 0, 0, 0);
}

Matrix* matrix_rows(const Matrix* m, int from, int to) {
    int rows = m->rows - from - (m->rows - to);
    int cols = m->cols;
    Matrix* rm = matrix_zeroes(rows, cols);
    rm->numbers = &m->numbers[from * m->cols];

    return rm;
}

double matrix_get(const Matrix* m, size_t row, size_t col) {
    return m->numbers[row * m->cols + col];
}

void matrix_set(Matrix* m, size_t row, size_t col, double value) {
    m->numbers[row * m->cols + col] = value;
}

void matrix_print(const Matrix* m) {
    int n = 0;
    for (int row = 0; row < m->rows; row++) {
        for (int col = 0; col < m->cols; col++) {
            printf("%0.3lf ", m->numbers[n]);
            n++;
        }
        printf("\n");
    }
}

Matrix* matrix_mult(const Matrix* m_a, const Matrix* m_b) {
    if (m_a->cols != m_b->rows) {
        return NULL;
    }

    Matrix* m_r = matrix_zeroes(m_a->rows, m_b->cols);
    for (int row = 0; row < m_a->rows; row++) {
        for (int col_in_b = 0; col_in_b < m_b->cols; col_in_b++) {
            double dot_product = 0;
            for (int col = 0; col < m_a->cols; col++) {
                double a = matrix_get(m_a, row, col);
                double b = matrix_get(m_b, col, col_in_b);
                dot_product += a * b;
            }
            matrix_set(m_r, row, col_in_b, dot_product);
        }
    }

    return m_r;
}

size_t matrix_rows_num(const Matrix* m) {
    return m->rows;
}

size_t matrix_cols_num(const Matrix* m) {
    return m->cols;
}

void matrix_drop(Matrix* m) {
    free(m->numbers);
    m->cols = 0;
    m->rows = 0;
    free(m);
}

Matrix* matrix_from_file(FILE* fid) {
    Matrix* m = matrix_empty();

    m->rows = 0;

    char* line = NULL;
    size_t buf_size = 0;
    while (!feof(fid)) {
        int n = getline(&line, &buf_size, fid);
        if (n != -1 && strncmp(line, "\n", 1) != 0) {
            m->rows++;
        }
    };

    fseek(fid, 0, 0);

    m->cols = 0;
    getline(&line, &buf_size, fid);
    char* token = strtok(line, " \n");
    while (token != NULL) {
        m->cols++;
        token = strtok(NULL, " \n");
    }

    fseek(fid, 0, 0);

    double* numbers = malloc(m->cols * m->rows * sizeof(double));
    int numberc = 0;
    int L = 0;
    while (!feof(fid)) {
        int n = getline(&line, &buf_size, fid);
        int cols = 0;
        L++;

        if (n != -1 && strcmp(line, "\n") != 0) {
            token = strtok(line, " \n");
            while (token != NULL) {
                char* end = 0;
                numbers[numberc] = strtod(token, &end);
                numberc++;
                cols++;
                token = strtok(NULL, " \n");
            }

            if (cols != m->cols) {
                fprintf(
                    stderr,
                    "Matrix rows must be equal length (line %d)\n",
                    L
                );
                return NULL;
            }
        }
    }

    m->numbers = numbers;
    
    free(line);

    return m;
}

void matrix_write_to_file(const Matrix* m, FILE* fid) {
    int n = 0;
    for (int row = 0; row < m->rows; row++) {
        for (int col = 0; col < m->cols; col++) {
            fprintf(fid, "%lf ", m->numbers[n]);
            n++;
        }
        fprintf(fid, "\n");
    }
}
