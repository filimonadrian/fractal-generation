#include "tema1_par.h"

// read the arguments of the program
void get_args(int argc, char **argv)
{
	if (argc < 6) {
		printf("Numar insuficient de parametri:\n\t"
				"./tema1 fisier_intrare_julia fisier_iesire_julia "
				"fisier_intrare_mandelbrot fisier_iesire_mandelbrot numar_threaduri\n");
		exit(1);
	}

	in_filename_julia = argv[1];
	out_filename_julia = argv[2];
	in_filename_mandelbrot = argv[3];
	out_filename_mandelbrot = argv[4];
	P = atoi(argv[5]);

}

// read the input file
void read_input_file(char *in_filename, params* par)
{
	FILE *file = fopen(in_filename, "r");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de intrare!\n");
		exit(1);
	}

	fscanf(file, "%d", &par->is_julia);
	fscanf(file, "%lf %lf %lf %lf",
			&par->x_min, &par->x_max, &par->y_min, &par->y_max);
	fscanf(file, "%lf", &par->resolution);
	fscanf(file, "%d", &par->iterations);

	if (par->is_julia) {
		fscanf(file, "%lf %lf", &par->c_julia.a, &par->c_julia.b);
	}

	fclose(file);
}

// write the result in the output file
void write_output_file(char *out_filename, int **result, int width, int height)
{
	int i, j;

	FILE *file = fopen(out_filename, "w");
	if (file == NULL) {
		printf("Eroare la deschiderea fisierului de iesire!\n");
		return;
	}

	fprintf(file, "P2\n%d %d\n255\n", width, height);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			fprintf(file, "%d ", result[i][j]);
		}
		fprintf(file, "\n");
	}

	fclose(file);
}

// allocate memory for the result
int **allocate_memory(int width, int height)
{
	int **result;
	int i;

	result = malloc(height * sizeof(int*));
	if (result == NULL) {
		printf("Eroare la malloc!\n");
		exit(1);
	}

	for (i = 0; i < height; i++) {
		result[i] = malloc(width * sizeof(int));
		if (result[i] == NULL) {
			printf("Eroare la malloc!\n");
			exit(1);
		}
	}

	return result;
}

// free the allocated memory
void free_memory(int **result, int height)
{
	int i;

	for (i = 0; i < height; i++) {
		free(result[i]);
	}
	free(result);
}

// run julia algorithm
void run_julia(params *par, int **result, int width, int height, int thread_id)
{
	int w, h;
	int start = thread_id * width / P;
	int end = MIN(((thread_id + 1) * width / P), width);

	for (w = 0; w < width; w++) {
		for (h = start; h < end; h++) {
			int step = 0;
      complex z = { .a = w * par->resolution + par->x_min,
              .b = h * par->resolution + par->y_min };

      while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par->iterations) {
        complex z_aux = { .a = z.a, .b = z.b };

        z.a = pow(z_aux.a, 2) - pow(z_aux.b, 2) + par->c_julia.a;
        z.b = 2 * z_aux.a * z_aux.b + par->c_julia.b;
        step++;
      }
			result[h][w] = step % 256;
		}
	}
}

// run mandelbrot algorithm
void run_mandelbrot(params *par, int **result, int width, int height, int thread_id)
{
	int w, h;
	int start = thread_id * width / P;
	int end = MIN(((thread_id + 1) * width / P), width);

	for (w = 0; w < width; w++) {
		for (h = start; h < end; h++) {
			complex c = { .a = w * par->resolution + par->x_min,
							.b = h * par->resolution + par->y_min };
			complex z = { .a = 0, .b = 0 };
			int step = 0;

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par->iterations) {
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2.0) - pow(z_aux.b, 2.0) + c.a;
				z.b = 2.0 * z_aux.a * z_aux.b + c.b;

				step++;
			}

			result[h][w] = step % 256;
		}
	}
}

	int width, height;
	int width_mandelbrot, height_mandelbrot;
	params par;
	int **result, **result_mandelbrot;

// transform the result from mathematical coordinates in screen coordinates
void transform_coordinates(int **result, int height, int thread_id) {
	
	int i = 0;
	int len = height/2;
	int start = thread_id * len / P;
	int end = MIN((thread_id +1) * len / P, len);
	
	for (i = start; i < end; i++) {
		int *aux = result[i];
		result[i] = result[height - i - 1];
		result[height - i - 1] = aux;
	}
}

// thread function where read, alloc, calculate, clean and write the result
void *generate_images(void *arg) {

	int thread_id = (long)arg;

	// read the input file and initialize the values in the thread 0
	if (thread_id == 0) {
		read_input_file(in_filename_julia, &par);
		width = (par.x_max - par.x_min) / par.resolution;
		height = (par.y_max - par.y_min) / par.resolution;
		result = allocate_memory(width, height);
	}

	// wait untill the values are read and memory is allocated
	pthread_barrier_wait(&barrier);

	// run julia algorithm
  run_julia(&par, result, width, height, thread_id);
	pthread_barrier_wait(&barrier);

	// tranform the coordinates of the result in screen-coordinates
	transform_coordinates(result, height, thread_id);
	pthread_barrier_wait(&barrier);
	
	// write in the file and free the memory for the first algorithm
	if (thread_id == 0) {
		write_output_file(out_filename_julia, result, width, height);
		free_memory(result, height);
	}

	// if is more than 1 thread
	// read in parallel and allocate memory for the second algorithm
	if (MAX(0, P - 1) == thread_id) {
		// clean the values for the reused struct
		memset(&par, 0, sizeof(params));
		
		read_input_file(in_filename_mandelbrot, &par);
		width_mandelbrot = (par.x_max - par.x_min) / par.resolution;
		height_mandelbrot = (par.y_max - par.y_min) / par.resolution;
		result_mandelbrot = allocate_memory(width_mandelbrot, height_mandelbrot);
	}

	// wait untill the values are read and memory is allocated
	pthread_barrier_wait(&barrier);
	
	// run mandelbrot algorithm
	run_mandelbrot(&par, result_mandelbrot, width_mandelbrot, height_mandelbrot, thread_id);
	pthread_barrier_wait(&barrier);

	// tranform the coordinates of the result in screen-coordinates
	transform_coordinates(result_mandelbrot, height_mandelbrot, thread_id);
	pthread_barrier_wait(&barrier);
	
	// first or last thread will write the result in file and free the memory
	if (MAX(0, P - 1) == thread_id) {
		write_output_file(out_filename_mandelbrot, result_mandelbrot, width_mandelbrot, height_mandelbrot);
		free_memory(result_mandelbrot, height_mandelbrot);
	}

	pthread_exit(NULL);

}

int main(int argc, char *argv[])
{
	int ret = 0;

	// read the arguments of the program
	get_args(argc, argv);

	// declare the threads and init the barrier
	pthread_t tid[P];
	ret = pthread_barrier_init(&barrier, NULL, P);
	if (ret) {
		printf("Can't init the barrier!\n");
	}

	// create the threads
	for (long id = 0; id < P; id++) {
		ret = pthread_create(&tid[id], NULL, generate_images, (void*)id);
		if (ret) {
			printf("Can't create %ldth thread\n", id);
		}
	}

	for (int i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}

	pthread_barrier_destroy(&barrier);

	return 0;
}
