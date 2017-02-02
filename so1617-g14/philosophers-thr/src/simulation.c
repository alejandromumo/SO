/**
 *  \brief Civilized philosophers buffet
 *
 * Simulation of SO's second assignment.
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include "parameters.h"
#include "dining-room.h"
#include "logger.h"
#include <pthread.h> /* added */

/* Global variable */
Simulation* sim;

/* internal functions */
static void help(char* prog);
static void processArgs(Parameters *params, int argc, char* argv[]);
static void showParams(Parameters *params);
static void *philosopher(void *pid); /* added */
static void *waiter(); /* added */

int main(int argc, char* argv[])
{
	// default parameter values:
	Parameters params = {5,10,100,3,2,10,10,20,50,10,15};
	processArgs(&params, argc, argv);
	showParams(&params);
	printf("<press RETURN>");
	getchar();

	sim = initSimulation(NULL, &params);
	logger(sim);

	/**
 	* launch threads/processes for philosophers and waiter
 	*/
 	pthread_t pthr[sim->params->NUM_PHILOSOPHERS], wthr[1];
	int i, ids[sim->params->NUM_PHILOSOPHERS];
	srand((int)getpid());

	/* launching the waiter */
	//printf("Launching 1 waiter thread\n");

	if(pthread_create(&wthr[0], NULL, waiter, NULL) != 0){
		fprintf(stderr, "waiter\n");
		perror("error on launching the waiter thread\n");
	}
	
	/* launching the philosophers */
	//printf("Launching %d philosophers threads\n", sim->params->NUM_PHILOSOPHERS);

	for(i = 0; i < sim->params->NUM_PHILOSOPHERS; i++){
		ids[i] = i;
		if(pthread_create(&pthr[i], NULL, philosopher, &ids[i]) != 0){
			fprintf(stderr, "Philosopher %d\n", i);
			perror("error on launching the philosopher thread\n");
		}
	}

	/* end */
	
	/**
	 * Wait for the death of all philosophers, and request and wait for waiter dead.
	 */
	for(i = 0; i < sim->params->NUM_PHILOSOPHERS; i++){
		if(pthread_join(pthr[i], NULL) != 0){
			fprintf(stderr, "Philosopher %d\n", i);
			perror("error on waiting for the philosopher thread to conclude\n");
		}
	}

	if(pthread_join(wthr[0], NULL) != 0){
		fprintf(stderr, "Waiter\n");
		perror("error on waiting for the waiter thread to conclude\n");
	}
	logger(sim);
	/* end */

	return 0;
}

static void *philosopher(void *pid){
	int id = *((int *)pid);
	philosopher_lifecycle(id);
	return NULL;
}

static void *waiter(){
	waiter_lifecycle();
	return NULL;
}

Simulation* initSimulation(Simulation* sim, Parameters* params)
{
	assert(params != NULL);

	int i;
	Simulation* result = sim;
	if (result == NULL)
		result = (Simulation*)mem_alloc(sizeof(Simulation));

	// simulation parameters:
	result->params = (Parameters*)mem_alloc(sizeof(Parameters));
	memcpy(result->params, params, sizeof(Parameters));

	// default DiningRoom values:
	result->diningRoom = (DiningRoom*)mem_alloc(sizeof(DiningRoom));
	DiningRoom s = {params->NUM_PIZZA, params->NUM_SPAGHETTI, params->NUM_FORKS, params->NUM_KNIVES, 0, 0, 0, 0, params->NUM_PHILOSOPHERS};
	memcpy(result->diningRoom, &s, sizeof(DiningRoom));

	// Philosopher:
	Philosopher p = {P_BIRTH,P_NONE,{P_NOTHING,P_NOTHING}};
	result->philosophers = (Philosopher**)mem_alloc(params->NUM_PHILOSOPHERS*sizeof(Philosopher*));
	for(i = 0; i < params->NUM_PHILOSOPHERS; i++)
	{
		result->philosophers[i] = (Philosopher*)mem_alloc(sizeof(Philosopher));
		memcpy(result->philosophers[i], &p, sizeof(Philosopher));
	}

	// Waiter:
	Waiter w = {W_NONE,W_INACTIVE,W_INACTIVE,W_INACTIVE};
	result->waiter = (Waiter*)mem_alloc(sizeof(Waiter));
	memcpy(result->waiter, &w, sizeof(Waiter));

	return result;
}

/*********************************************************************/
// No need to change remaining code!

static void help(char* prog)
{
	assert(prog != NULL);

	printf("\n");
	printf("Usage: %s [OPTION] ...\n", prog);
	printf("\n");
	printf("Options:\n");
	printf("\n");
	printf("  -h, --help               show this help\n");
	printf("  -n, --num-philosophers   set number of philosophers (default is 5)\n");
	printf("  -l, --min-life   set minimum number of iterations of philosophers life cycle (default is 10)\n");
	printf("  -L, --max-life   set maximum number of iterations of philosophers life cycle (default is 100)\n");
	printf("  -f, --num-forks   set number of forks (default is 3)\n");
	printf("  -k, --num-knives   set number of knives (default is 2)\n");
	printf("  -p, --pizza   set number of pizza meals in each replenish operation (default is 10)\n");
	printf("  -s, --spaghetti   set number of spaghetti meals in each replenish operation (default is 10)\n");
	printf("  -t, --think-time   set maximum milliseconds for thinking (default is 20)\n");
	printf("  -c, --choose-pizza-prob   set probability to choose a pizza meal against a spaghetti meal (default is 50)\n");
	printf("  -e, --eat-time   set maximum milliseconds for eating (default is 10)\n");
	printf("  -w, --wash-time   set maximum milliseconds for washing (default is 15)\n");
	printf("\n");
}

static void processArgs(Parameters *params, int argc, char* argv[])
{
	assert(params != NULL);
	assert(argc >= 0 && argv != NULL && argv[0] != NULL);

	static struct option long_options[] =
	{
		{"help",             no_argument,       NULL, 'h' },
		{"num-philosophers", required_argument, NULL, 'n' },
		{"min-life",         required_argument, NULL, 'l' },
		{"max-life",         required_argument, NULL, 'L' },
		{"num-forks",        required_argument, NULL, 'f' },
		{"num-knives",       required_argument, NULL, 'k' },
		{"pizza",            required_argument, NULL, 'p' },
		{"spaghetti",        required_argument, NULL, 's' },
		{"think-time",       required_argument, NULL, 't' },
		{"choose-pizza-prob",required_argument, NULL, 'c' },
		{"eat-time",         required_argument, NULL, 'e' },
		{"wash-time",        required_argument, NULL, 'w' },
		{0,          0,                 NULL,  0 }
	};
	int op=0;

	while (op != -1)
	{
		int option_index = 0;

		op = getopt_long(argc, argv, "hn:l:L:f:k:p:s:t:c:e:w:", long_options, &option_index);
		int n; // integer number
		switch (op)
		{
			case -1:
				break;

			case 'h':
				help(argv[0]);
				exit(EXIT_SUCCESS);

			case 'n':
				n = atoi(optarg);
				if (n < 1)
				{
					fprintf(stderr, "ERROR: invalid number of philosophers \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->NUM_PHILOSOPHERS = n;
				break;

			case 'l':
				n = atoi(optarg);
				if (n < 0 || (n == 0 && strcmp(optarg, "0") != 0))
				{
					fprintf(stderr, "ERROR: invalid minimum philosophers life \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->PHILOSOPHER_MIN_LIVE = n;
				break;

			case 'L':
				n = atoi(optarg);
				if (n < 0 || n < params->PHILOSOPHER_MIN_LIVE || (n == 0 && strcmp(optarg, "0") != 0))
				{
					fprintf(stderr, "ERROR: invalid maximum philosophers life \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->PHILOSOPHER_MAX_LIVE = n;
				break;

			case 'f':
				n = atoi(optarg);
				if (n < 2)
				{
					fprintf(stderr, "ERROR: invalid number of forks \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->NUM_FORKS = n;
				break;

			case 'k':
				n = atoi(optarg);
				if (n < 1)
				{
					fprintf(stderr, "ERROR: invalid number of knives \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->NUM_KNIVES = n;
				break;

			case 'p':
				n = atoi(optarg);
				if (n < 1)
				{
					fprintf(stderr, "ERROR: invalid number of pizza meals \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->NUM_PIZZA = n;
				break;

			case 's':
				n = atoi(optarg);
				if (n < 1)
				{
					fprintf(stderr, "ERROR: invalid number of spaghetti meals \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->NUM_SPAGHETTI = n;
				break;

			case 't':
				n = atoi(optarg);
				if (n < 0 || (n == 0 && strcmp(optarg, "0") != 0))
				{
					fprintf(stderr, "ERROR: invalid think time \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->THINK_TIME = n;
				break;

			case 'c':
				n = atoi(optarg);
				if (n < 0 || n > 100 || (n == 0 && strcmp(optarg, "0") != 0))
				{
					fprintf(stderr, "ERROR: invalid percentage for choosing pizza against spaghetti meals \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->CHOOSE_PIZZA_PROB = n;
				break;

			case 'e':
				n = atoi(optarg);
				if (n < 0 || (n == 0 && strcmp(optarg, "0") != 0))
				{
					fprintf(stderr, "ERROR: invalid eat time \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->EAT_TIME = n;
				break;

			case 'w':
				n = atoi(optarg);
				if (n < 0 || (n == 0 && strcmp(optarg, "0") != 0))
				{
					fprintf(stderr, "ERROR: invalid wash time \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				params->WASH_TIME = n;
				break;

			default:
				help(argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}

	if (optind < argc)
	{
		fprintf(stderr, "ERROR: invalid extra arguments\n");
		exit(EXIT_FAILURE);
	}

}

static void showParams(Parameters *params)
{
	assert(params != NULL);

	printf("\n");
	printf("Simulation parameters:\n");
	printf("  --num-philosophers: %d\n", params->NUM_PHILOSOPHERS);
	printf("  --min-life: %d\n", params->PHILOSOPHER_MIN_LIVE);
	printf("  --max-life: %d\n", params->PHILOSOPHER_MAX_LIVE);
	printf("  --num-forks: %d\n", params->NUM_FORKS);
	printf("  --num-knives: %d\n", params->NUM_KNIVES);
	printf("  --pizza: %d\n", params->NUM_PIZZA);
	printf("  --spaghetti: %d\n", params->NUM_SPAGHETTI);
	printf("  --think-time: %d\n", params->THINK_TIME);
	printf("  --choose-pizza-prob: %d\n", params->CHOOSE_PIZZA_PROB);
	printf("  --eat-time: %d\n", params->EAT_TIME);
	printf("  --wash-time: %d\n", params->WASH_TIME);
	printf("\n");
}

/**
 * Memory error is not recoverable.
 */
void* mem_alloc(int size)
{
	void* result = malloc(size);
	if (result == NULL)
	{
		fprintf(stderr, "ERROR: no memory!\n");
		exit(EXIT_FAILURE);
	}
	return result;
}
