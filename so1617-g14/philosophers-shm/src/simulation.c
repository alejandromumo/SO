/**
 *  \brief Civilized philosophers buffet
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "parameters.h"
#include "dining-room.h"
#include "waiter.h"
#include "logger.h"
#include "utils.h"

/* internal functions */
static void help(char* prog);
static void args_parse(Parameters *params, int argc, char* argv[]);
static void show_params(Parameters *params);

static void simulation_fork_to_philosopher(pid_t *pidp, int i, Simulation *s);
static void simulation_fork_to_waiter(pid_t *pidp, Waiter *w, Simulation *s);

void simulation_debug(Simulation *s)
{
    printf("Simulation:\n");
    printf("\tAddress: %p\n", s);
    printf("\tPointer Size: %lu\n", sizeof(s));
    printf("\tStruct Size: %lu\n", sizeof(Simulation));

    printf("Params:\n");
    printf("\tAddress: %p\n", s->params);
    printf("\tNUM_PHILOSOPHERS: %d\n", s->params->NUM_PHILOSOPHERS);
    printf("\tPHILOSOPHER_MIN_LIVE: %d\n", s->params->PHILOSOPHER_MIN_LIVE);
    printf("\tPHILOSOPHER_MAX_LIVE: %d\n", s->params->PHILOSOPHER_MAX_LIVE);
    printf("\tNUM_FORKS: %d\n", s->params->NUM_FORKS);
    printf("\tNUM_KNIVES: %d\n", s->params->NUM_KNIVES);
    printf("\tNUM_PIZZA: %d\n", s->params->NUM_PIZZA);
    printf("\tNUM_SPAGHETTI: %d\n", s->params->NUM_SPAGHETTI);
    printf("\tTHINK_TIME: %d\n", s->params->THINK_TIME);
    printf("\tCHOOSE_PIZZA_PROB: %d\n", s->params->CHOOSE_PIZZA_PROB);
    printf("\tEAT_TIME: %d\n", s->params->EAT_TIME);
    printf("\tWASH_TIME: %d\n", s->params->WASH_TIME);

    printf("Dining Room:\n");
    printf("\tAddress: %p\n", s->diningRoom);
    printf("\tpizza: %d\n", s->diningRoom->pizza);
    printf("\tspaghetti: %d\n", s->diningRoom->spaghetti);
    printf("\tcleanForks: %d\n", s->diningRoom->cleanForks);
    printf("\tcleanKnives: %d\n", s->diningRoom->cleanKnives);
    printf("\tdirtyForks: %d\n", s->diningRoom->dirtyForks);
    printf("\tdirtyKnives: %d\n", s->diningRoom->dirtyKnives);
    printf("\tdirtyForksInWaiter: %d\n", s->diningRoom->dirtyForksInWaiter);
    printf("\tdirtyKnivesInWaiter: %d\n", s->diningRoom->dirtyKnivesInWaiter);

    printf("Philosophers:\n");
    printf("\tAddress %p\n", s->philosophers);
    int i;
    for (i = 0; i < s->params->NUM_PHILOSOPHERS; i++)
    {
        printf("\tPhilosopher %d\n", i);
        printf("\t\tAddress: %p\n", s->philosophers[i]);
        printf("\t\tstate: %d\n", (int) s->philosophers[i]->state);
        printf("\t\tmeal: %d\n", (int) s->philosophers[i]->meal);
        printf("\t\tcutlery[0]: %d\n", (int) s->philosophers[i]->cutlery[0]);
        printf("\t\tcutlery[1]: %d\n", (int) s->philosophers[i]->cutlery[1]);
    }

    printf("Waiter:\n");
    printf("\tAddress: %p\n", s->waiter);
    printf("\tstate: %d\n", (int) s->waiter->state);
    printf("\treqCutlery: %d\n", (int) s->waiter->reqCutlery);
    printf("\treqPizza: %d\n", (int) s->waiter->reqPizza);
    printf("\treqSpaghetti: %d\n", (int) s->waiter->reqSpaghetti);

    printf("\n\n=====================================\n\n");
}

int main(int argc, char* argv[])
{
    // Handle parameters
    Parameters params = {5,10,100,3,2,10,10,20,50,10,15};
    args_parse(&params, argc, argv);
    show_params(&params);
    printf("<press RETURN>");
    getchar();

    // Map simulation in shared memory
    Simulation *s = simulation_share(simulation_new(&params));

    // Bootstrap the simulation
    pid_t *pid = memory_allocate(params.NUM_PHILOSOPHERS + 1);
    logger(s);
    simulation_start(s, pid);
    simulation_stop(s, pid);
    logger(s);

    // Unmap simulation in shared memory
    simulation_unshare(s);

    return 0;
}

Simulation *simulation_new(Parameters *params)
{
    assert(params != NULL);

    Simulation *s = (Simulation*) memory_allocate(sizeof(Simulation));

    s->params = (Parameters*) memory_allocate(sizeof(Parameters));
    memcpy(s->params, params, sizeof(Parameters));

    s->diningRoom = dining_room_new(params->NUM_PIZZA, params->NUM_SPAGHETTI, params->NUM_FORKS, params->NUM_KNIVES, 0, 0, 0, 0);

    s->philosophers = (Philosopher**) memory_allocate(params->NUM_PHILOSOPHERS * sizeof(Philosopher*));
    int i;
    for(i = 0; i < params->NUM_PHILOSOPHERS; i++)
        s->philosophers[i] = philosopher_new();

    s->waiter = waiter_new();

    return s;
}

Simulation *simulation_share(Simulation *s)
{
    int n = s->params->NUM_PHILOSOPHERS;
    size_t length = sizeof(Simulation) + sizeof(Parameters) + sizeof(DiningRoom) + n*sizeof(Philosopher*) + n*sizeof(Philosopher) + sizeof(Waiter);

    // Get contiguous block of shared memory for the simulation
    Simulation *sm = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (sm == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Deep copy simulation
    char *p = (char *) sm;
    p += sizeof(Simulation);

    sm->params = (Parameters*) p;
    memcpy(sm->params, s->params, sizeof(Parameters));
    p += sizeof(Parameters);

    sm->diningRoom = (DiningRoom*) p;
    memcpy(sm->diningRoom, s->diningRoom, sizeof(DiningRoom));
    p += sizeof(DiningRoom);

    sm->philosophers = (Philosopher**) p;
    p += (n * sizeof(Philosopher*));

    int i;
    for (i = 0; i < n; i++)
    {
        sm->philosophers[i] = (Philosopher*) p;
        memcpy(sm->philosophers[i], s->philosophers[i], sizeof(Philosopher));
        p += sizeof(Philosopher);
    }

    sm->waiter = (Waiter*) p;
    memcpy(sm->waiter, s->waiter, sizeof(Waiter));

    return sm;
}

void simulation_unshare(Simulation *s)
{
    int n = s->params->NUM_PHILOSOPHERS;
    size_t length = sizeof(Simulation) + sizeof(Parameters) + sizeof(DiningRoom) + n*sizeof(Philosopher*) + n*sizeof(Philosopher) + sizeof(Waiter);

    if (munmap(s, length) != 0)
    {
        perror("munmap");
        exit(EXIT_FAILURE);
    }
}

static void simulation_fork_to_philosopher(pid_t *pidp, int i, Simulation *s)
{
    pid_t pid = fork();

    switch (pid)
    {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0:
            srand((int) getpid());
            philosopher_lifecycle(s->philosophers[i], s);
            return;

        default:
            *pidp = pid;
            return;
    }
}

static void simulation_fork_to_waiter(pid_t *pidp, Waiter *w, Simulation *s)
{
    pid_t pid = fork();

    switch (pid)
    {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0:
            srand((int) getpid());
            waiter_lifecycle(w, s);
            return;

        default:
            *pidp = pid;
            return;
    }
}

bool simulation_are_philosophers_dead(Simulation *s)
{
    int i;
    for (i = 0; i < s->params->NUM_PHILOSOPHERS; i++)
        if (s->philosophers[i]->state != P_DEAD)
            return false;
    return true;
}

/**
 * Launch processes for philosophers and waiter
 */
void simulation_start(Simulation *s, pid_t *pid)
{
    assert(s != NULL);

    int n = s->params->NUM_PHILOSOPHERS;
    int i;

    // Spawn the philosophers' processes
    for (i = 0; i < n; i++)
        simulation_fork_to_philosopher(&pid[i], i, s);

    // Spawn the waiter's process
    simulation_fork_to_waiter(&pid[i], s->waiter, s);
}

/**
 * Wait for the death of all philosophers, and request and wait for waiter dead
 */
void simulation_stop(Simulation *s, pid_t *pid)
{
    assert(s != NULL);

    int n = s->params->NUM_PHILOSOPHERS;
    int i;

    // Wait for the philosophers' processes to terminate
    for (i = 0; i < n; i++)
        waitpid(pid[i], NULL, 0);

    // Wait for the waiter's process to terminate
    waitpid(pid[i], NULL, 0);
}

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

static void args_parse(Parameters *params, int argc, char* argv[])
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

    int op = 0;

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

static void show_params(Parameters *params)
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
