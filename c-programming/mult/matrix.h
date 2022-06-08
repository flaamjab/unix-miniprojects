#ifndef MATRIX_H
#define MATRIX_H

#include <stdlib.h>

typedef struct _Matrix Matrix;

Matrix* matrix_new(
    double* numbers,
    size_t cnumbers,
    size_t rows,
    size_t cols
);

Matrix* matrix_zeroes(size_t rows, size_t cols);
Matrix* matrix_empty();
void matrix_drop(Matrix* m);
Matrix* matrix_from_file(FILE* fid);
void matrix_write_to_file(const Matrix* m, FILE* fid);

Matrix* matrix_rows(const Matrix* m, int from, int to);
Matrix* matrix_mult(const Matrix* m_a, const Matrix* m_b);
size_t matrix_rows_num(const Matrix* m);
size_t matrix_cols_num(const Matrix* m);
double* matrix_raw(const Matrix* m);
double matrix_get(const Matrix* m, size_t row, size_t col);
void matrix_set(Matrix* m, size_t row, size_t col, double value);
void matrix_print(const Matrix* m);

#endif
