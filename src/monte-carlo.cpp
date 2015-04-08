#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define PI(hits_no, n) hits_no / (double) n * 4
#define SPEEDUP_FACTOR(t1, t2) t1 / t2
#define EFFICIENCY_FACTOR(t1, t2, p) t1 / ((double) p * t2)

pthread_barrier_t barrier;

struct case_ret {
	double pi; // obliczona liczba pi
	double time; // zmierzony  czas
};

double countMonteCarloPi(int n, int p);
int countHits(int n);
void setSrand();
double randomNumber(double min, double max);
void *thread_routine(void *ptr);
void setThreadsArgs(int n,  int arguments[], int p);
void runtTest(int n, int i, int p[], int p_size);
case_ret get_mean_call_time(unsigned int calls);




int main(int argc, char **argv)
{
	int opt;
	char *optstring = "n:i:p:v";

	int n = 0;
	int i = 0;
	char *p_string;
	char *p_token;
	int *p = NULL;
	int p_size = 0;
	char verbose = 0;

	int j;

	while ((opt = getopt(argc, argv, optstring)) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
				break;

			case 'i':
        i = atoi(optarg);
				break;

			case 'p':
				p_string = optarg;

				while ((p_token = strtok(p_string, ","))) {
					p = (int *) realloc(p,
						++p_size * sizeof (int));
					p[p_size - 1] = atoi(p_token);
					p_string = NULL;
				}
				break;


			case 'v':
				verbose = 1;
				break;

			default:
				printf("cos poszlo nie tak");

        if (p) {
					free(p);
				}
				return EXIT_FAILURE;
		}
	}

	if (verbose) {
		puts("Printing parameter values:");
		printf("Parameter n = %d\n", n);
		printf("Parameter i = %d\n", i);
		printf("Parameter p = ");

		for (j = 0; j < p_size; j++) {
			printf("%d", p[j]);
			if (j < p_size - 1) {
				printf(", ");
			}
		}
		puts("");
	}

	runtTest(n, i, p, p_size);

	free(p);
	return EXIT_SUCCESS;
}


double countMonteCarloPi(int n, int p)
{
	int hits_no = 0;
	setSrand();

	pthread_barrier_init(&barrier, NULL, p + 1);

	// tablica argumentow przekazywanych do kolejnych watkow:
	int arguments[p];
	setThreadsArgs(n, arguments, p);

	// tablica identyfikatorow tworzonych watkow:
	pthread_t threads[p];

	int *return_value;

	// inicjowanie watkow:
	int i;
	for (i = 0; i < p; i++) {
		bool thread_failed_on_create = (bool) pthread_create(
			&threads[i],
			NULL,
			thread_routine, // nazwa funkcji wykonywanej przez watek
			&arguments[i]
		);

		if (thread_failed_on_create) {
			puts("Failed to create thread!");
			return EXIT_FAILURE;
		}
	}


	pthread_barrier_wait(&barrier);


	// zebranie watkow
	for (i = 0; i < p; i++) {
		bool failed_on_thread_joint = (bool) pthread_join(
			threads[i],
			(void **) &return_value // adres w pamieci pod ktorym jest zapisany inny adres
		);

		if (failed_on_thread_joint) {
			puts("Thread could not be joined!");
			return EXIT_FAILURE;
		} else {
			hits_no += *return_value;
			free(return_value);
		}
	}

	return PI(hits_no, n);
}


int countHits(int n)
{
	double x_coordinate;
	double y_coordinate;
	double random_radius;

	int hits_no = 0;

	int i = 0;
	for (i; i < n; i++) {
		x_coordinate = randomNumber(-1, 1);
		y_coordinate = randomNumber(-1, 1);

		random_radius = sqrt( pow(x_coordinate, 2) + pow(y_coordinate, 2) );

		if (random_radius <= 1) {
			hits_no += 1;
		}
	}

	return hits_no;
}


void setSrand()
{
	struct timeval tp;
	gettimeofday(&tp, NULL);
	srand(tp.tv_sec  + tp.tv_usec);
}


double randomNumber(double min, double max)
{
    double random01 = (double) rand() / RAND_MAX;
    return min + random01 * (max - min);
}


void* thread_routine(void *ptr) {
	int init_value = *( (int *) ptr);
	int *return_value;

	// alokujemy pamiec na stercie, by byla dostepna po zakonczeniu watku:
	return_value = (int *) malloc(sizeof (int));

	*return_value = countHits(init_value);

	pthread_barrier_wait(&barrier);

	// zwracany adres pod ktorym jest wartosc zwracana przez watek:
	pthread_exit(return_value);
}


void setThreadsArgs(int n,  int arguments[], int p)
{
	int i;

	int quotient = (int) ( n / p);
	int rest = n % p;

	for (i = 0; i < p; i++) {
		if (i == 0) {
			arguments[i] = quotient + rest;
		} else {
			arguments[i] = quotient;
		}
	}
}


case_ret get_mean_call_time(unsigned int calls, int n, int p) {
	struct timespec t0;
	struct timespec t1;
	unsigned int i;

	case_ret ret;
	ret.pi = 0;

	clock_gettime(CLOCK_MONOTONIC, &t0);

	for (i = 0; i < calls; i++) {
		ret.pi += countMonteCarloPi(n, p);
	}
	ret.pi /= (double) calls;

	clock_gettime(CLOCK_MONOTONIC, &t1);

	ret.time =  (double) ((t1.tv_sec - t0.tv_sec)
			+ (t1.tv_nsec - t0.tv_nsec) * 1e-9)
		/ calls;

	return ret;
}


void runtTest(int n, int i, int p[], int p_size) {
	case_ret ret_all;
	case_ret ret_one;

	ret_one = get_mean_call_time(i, n, 1);

	puts("");
	printf("n = %d\n", n);
	// printf("p  a      t      wp   we \n");
	printf("p  a             t             twp           twe \n");
	// printf("--------------------------\n");
	printf("---------------------------------------------------------\n");

	int k = 0;
	for (k; k < p_size; k++) {
		ret_all = get_mean_call_time(i, n, p[k]);

		// printf("%d  %.3f  %.3f  %.1f  %.1f \n",
		printf("%d  %e  %e  %e  %e \n",
			p[k], ret_all.pi, ret_all.time,
				SPEEDUP_FACTOR(ret_one.time, ret_all.time),
					EFFICIENCY_FACTOR(ret_one.time, ret_all.time, p[k]));
	}

	puts("");

}
