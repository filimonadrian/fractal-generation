#include "tema1_par.h"

// citeste argumentele programului
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

// citeste fisierul de intrare
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

// scrie rezultatul in fisierul de iesire
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

// aloca memorie pentru rezultat
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

// elibereaza memoria alocata
void free_memory(int **result, int height)
{
	int i;

	for (i = 0; i < height; i++) {
		free(result[i]);
	}
	free(result);
}

// ruleaza algoritmul Julia
// void *run_julia(void *arg)
// void run_julia(params par, int **result, int width, int height, void *arg)
void *run_julia(void *arg)
{
	arguments *args = (arguments *)arg;
	params *par = (params*)args->par;
	int **result = args->result;
	int width = args->width;
	int height = args->height;
	int w, h, i;
	printf("width: %d height: %d", width, height);
	printf("x_min: %lf x_max: %lf y_min: %lf y_max: %lf resolution: %lf\n", par->x_min, par->x_max, par->x_min, par->y_max, par->resolution);

	// for (int i = 0; i < 2000; i++) {
	// 	for (int j = 0; j < 2000; j++) {
	// 		printf("%d ", result[j][i]);
	// 	}
	// 	printf("\n");
	// }


// for (int i = 0; i < width; i++) {
// 	for (int j = 0; j < height; j++) {
// 		printf("h: %d w: %d val: %d\n", j, i, result[i][j]);
// 	}
// }

	for (w = 0; w < width; w++) {
		for (h = 0; h < height; h++) {
			int step = 0;
			complex z = { .a = w * par->resolution + par->x_min,
							.b = h * par->resolution + par->y_min };
//			printf("step: %d; z.a = %lf, z.b = %lf\n", step, z.a, z.b);

			while (sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)) < 2.0 && step < par->iterations) {
				// printf("While\n");
				// printf("cond1 %lf\n", sqrt(pow(z.a, 2.0) + pow(z.b, 2.0)));
				// printf("step = %d\n", step);
				complex z_aux = { .a = z.a, .b = z.b };

				z.a = pow(z_aux.a, 2) - pow(z_aux.b, 2) + par->c_julia.a;
				z.b = 2 * z_aux.a * z_aux.b + par->c_julia.b;

				step++;
			// printf("endwhile");
				
			}
			result[h][w] = step % 256;
			// printf("h: %d w: %d val: %d\n", h, w, result[h][w]);

			// printf("new_matrix: %d", result[h][w]);
			// printf("endforh");

		}
			// printf("\n");
	}

	// transforma rezultatul din coordonate matematice in coordonate ecran
	for (int i = 0; i < height / 2; i++) {
		int *aux = result[i];
		result[i] = result[height - i - 1];
		result[height - i - 1] = aux;
	}

	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	params *par = malloc (sizeof(params));

	int width, height;
	int **result;
	int thread_id[P];
	pthread_t tid[P];
	pthread_barrier_init(&barrier, NULL, P);

	// se citesc argumentele programului
	get_args(argc, argv);

	// Julia:
	// - se citesc parametrii de intrare
	// - se aloca tabloul cu rezultatul
	// - se ruleaza algoritmul
	// - se scrie rezultatul in fisierul de iesire
	// - se elibereaza memoria alocata
	read_input_file(in_filename_julia, par);

	width = (par->x_max - par->x_min) / par->resolution;
	height = (par->y_max - par->y_min) / par->resolution;

	result = allocate_memory(width, height);

	// put some values in the matrix
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < height; j++) {
			result[i][j] = 7;
		}
	}

	// malloc this struct and fill it with addresses
	// this arguments will be used by all threads in this form
	// no need to copy all elements for every thread
	arguments *args = malloc(sizeof(arguments));
	args->par = par;
	args->result = result;
	args->width = width;
	args->height = height;

	// printf("xmin din par este %lf\n", args->par->x_min);
	printf("%ld %ld %ld %ld\n", sizeof(par), sizeof(result), sizeof(width), sizeof(height));
	printf("%ld\n", sizeof(*args));
	// run_julia(args);

	// printf("Numarul de threaduri: %d\n", P);
	// se creeaza thread-urile
	int ret = 0;
	for (int i = 0; i < P; i++) {
		thread_id[i] = i;
		ret = pthread_create(&tid[i], NULL, run_julia, args);
		if (ret) {
			printf("Can't create %dth thread\n", i);
		}
	}
	for (int i = 0; i < P; i++) {
		pthread_join(tid[0], NULL);
	}

	write_output_file(out_filename_julia, result, width, height);
	free_memory(result, height);

	// // se asteapta thread-urile

	// // Mandelbrot:
	// // - se citesc parametrii de intrare
	// // - se aloca tabloul cu rezultatul
	// // - se ruleaza algoritmul
	// // - se scrie rezultatul in fisierul de iesire
	// // - se elibereaza memoria alocata
	// read_input_file(in_filename_mandelbrot, &par);

	// width = (par.x_max - par.x_min) / par.resolution;
	// height = (par.y_max - par.y_min) / par.resolution;

	// result = allocate_memory(width, height);
	// // seg fault at run_mandelbrot, idk why, probably reusage of arguments
	// run_mandelbrot(&par, result, width, height);
	// write_output_file(out_filename_mandelbrot, result, width, height);
	// free_memory(result, height);



	return 0;
}
