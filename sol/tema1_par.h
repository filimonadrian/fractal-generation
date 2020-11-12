#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

// a complex number
typedef struct _complex {
	double a;
	double b;
} complex;

// the parameters for one run
typedef struct _params {
	int is_julia, iterations;
	double x_min, x_max, y_min, y_max, resolution;
	complex c_julia;
} params;

typedef struct _arguments {
	params *par;
	int **result;
	int width;
	int height;
	int thread_id;
} arguments;

char *in_filename_julia;
char *in_filename_mandelbrot;
char *out_filename_julia;
char *out_filename_mandelbrot;
int P = 0;
pthread_barrier_t barrier;
int width, height;
int width_mandelbrot, height_mandelbrot;
params par;
int **result, **result_mandelbrot;
