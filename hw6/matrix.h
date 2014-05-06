typedef struct {
    int height;
    int width;
    float *data;
    int is_inverted;
} Matrix;

Matrix matrix_malloc(int height, int width);
void   matrix_free(Matrix matrix);

Matrix matrix_read(char *file_name);
void   matrix_print(Matrix matrix);
void   matrix_fprint(FILE *stream, Matrix matrix);
void   matrix_write(char *file_name, Matrix matrix);

float  matrix_get(Matrix matrix, int row, int col);
int    matrix_get_index(Matrix matrix, int row, int col);
void   matrix_get_submatrix(Matrix *submatrix, Matrix matrix, int start_row, int start_col);

void   matrix_set(Matrix *matrix, int row, int col, float val);
void   matrix_invert(Matrix *matrix);
void   matrix_chunk(Matrix *matrix, int sqrt_total_procs);

void   matrix_multiply(Matrix *result, Matrix a, Matrix b);
void   matrix_add(Matrix *d)