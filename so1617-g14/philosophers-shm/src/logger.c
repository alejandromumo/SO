/**
 *  \brief Logging module
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "logger.h"
#include "utils.h"

static void unsafe_logger(Simulation* sim);

/**
 * Change this function to get a safe access to shared simulation data!
 */
void logger(Simulation* sim)
{
    assert(sim != NULL);

    sem_t *mutex_logger = semaphore_open("/mutex_logger", 1);
    semaphore_wait(mutex_logger);
    unsafe_logger(sim);
    semaphore_post(mutex_logger);
}

/*********************************************************************/
// No need to change remaining code!

#ifdef NO_COLOR
#define BACKGROUND_WHITE ""
#define BLACK ""
#define RED ""
#define GREEN ""
#define YELLOW ""
#define BLUE ""
#define MAGENTA ""
#define END ""
#else
#define BACKGROUND_WHITE "\e[107m"
#define BLACK "\e[30m"
#define RED "\e[31m"
#define GREEN "\e[32m"
#define YELLOW "\e[33m"
#define BLUE "\e[34m"
#define MAGENTA "\e[35m"
#define END "\e[0m"
#endif

#if !defined(UTF8_SYMBOLS1) && !defined(UTF8_SYMBOLS2) && !defined(ASCII_SYMBOLS)
//#define ASCII_SYMBOLS
#define UTF8_SYMBOLS2
#endif

#ifdef UTF8_SYMBOLS2
#define EMPTY_SYMBOL "  "
#define HEADER_SPACES "  "
#define EATING_SYMBOL "\xE2\x9A\x87 "
#define HUNGRY_SYMBOL "\xE2\x98\xB9 "
#define FULL_SYMBOL "\xE2\x98\xBA "
#define DEAD_SYMBOL "\xE2\x9C\x9E "
#define WAIT_SYMBOL "\xE2\x99\xAB "
#define THINK_SYMBOL "\xE2\x98\xAF "
#define WASH_CUTLERY_SYMBOL "\xE2\x9C\x8D "
#define PIZZA_SYMBOL "\xE2\x97\x95 "
#define SPAGHETTI_SYMBOL "\xE2\x9B\x86 "
#endif

#ifdef UTF8_SYMBOLS1
#define EMPTY_SYMBOL " "
#define HEADER_SPACES ""
#define EATING_SYMBOL "\xE2\x9A\x87"
#define HUNGRY_SYMBOL "\xE2\x98\xB9"
#define FULL_SYMBOL "\xE2\x98\xBA"
#define DEAD_SYMBOL "\xE2\x9C\x9E"
#define WAIT_SYMBOL "\xE2\x99\xAB"
#define THINK_SYMBOL "\xE2\x98\xAF"
#define WASH_CUTLERY_SYMBOL "\xE2\x9C\x8D"
#define PIZZA_SYMBOL "\xE2\x97\x95"
#define SPAGHETTI_SYMBOL "\xE2\x9B\x86"
#endif

#ifdef ASCII_SYMBOLS
#define EMPTY_SYMBOL " "
#define HEADER_SPACES ""
#define EATING_SYMBOL "E"
#define HUNGRY_SYMBOL "H"
#define FULL_SYMBOL "F"
#define DEAD_SYMBOL "D"
#define WAIT_SYMBOL "W"
#define THINK_SYMBOL "T"
#define WASH_CUTLERY_SYMBOL "C"
#define PIZZA_SYMBOL "P"
#define SPAGHETTI_SYMBOL "S"
#endif

static int showHeader = 1;

static void invariantCheck(Simulation* sim)
{
    assert(sim != NULL);

    if (sim->params == NULL)
    {
        fprintf(stderr, "INVARIANT ERROR: sim->params not defined!!\n");
        exit(EXIT_FAILURE);
    }
    if (sim->diningRoom == NULL)
    {
        fprintf(stderr, "INVARIANT ERROR: sim->diningRoom not defined!!\n");
        exit(EXIT_FAILURE);
    }
    if (sim->philosophers == NULL)
    {
        fprintf(stderr, "INVARIANT ERROR: sim->philosophers not defined!!\n");
        exit(EXIT_FAILURE);
    }
    if (sim->waiter == NULL)
    {
        fprintf(stderr, "INVARIANT ERROR: sim->waiter not defined!!\n");
        exit(EXIT_FAILURE);
    }
    if (!(sim->diningRoom->pizza >= 0 && sim->diningRoom->pizza <= sim->params->NUM_PIZZA))
    {
        fprintf(stderr, "INVARIANT ERROR: Invalid buffet number of pizzas: %d!\n", sim->diningRoom->pizza);
        exit(EXIT_FAILURE);
    }
    if (!(sim->diningRoom->spaghetti >= 0 && sim->diningRoom->spaghetti <= sim->params->NUM_SPAGHETTI))
    {
        fprintf(stderr, "INVARIANT ERROR: Invalid buffet number of spaghetti: %d\n", sim->diningRoom->spaghetti);
        exit(EXIT_FAILURE);
    }
    if (!(sim->diningRoom->cleanForks >= 0 && sim->diningRoom->cleanForks <= sim->params->NUM_FORKS))
    {
        fprintf(stderr, "INVARIANT ERROR: Invalid buffet number of clean forks: %d\n", sim->diningRoom->cleanForks);
        exit(EXIT_FAILURE);
    }
    if (!(sim->diningRoom->cleanKnives >= 0 && sim->diningRoom->cleanKnives <= sim->params->NUM_KNIVES))
    {
        fprintf(stderr, "INVARIANT ERROR: Invalid buffet number of clean knives: %d\n", sim->diningRoom->cleanKnives);
        exit(EXIT_FAILURE);
    }
    if (!(sim->diningRoom->dirtyForks+sim->diningRoom->dirtyForksInWaiter >= 0 && sim->diningRoom->dirtyForks+sim->diningRoom->dirtyForksInWaiter <= sim->params->NUM_FORKS))
    {
        fprintf(stderr, "INVARIANT ERROR: Invalid buffet number of dirty forks: %d\n", sim->diningRoom->dirtyForks+sim->diningRoom->dirtyForksInWaiter);
        exit(EXIT_FAILURE);
    }
    if (!(sim->diningRoom->dirtyKnives+sim->diningRoom->dirtyKnivesInWaiter >= 0 && sim->diningRoom->dirtyKnives+sim->diningRoom->dirtyKnivesInWaiter <= sim->params->NUM_KNIVES))
    {
        fprintf(stderr, "INVARIANT ERROR: Invalid buffet number of dirty knives: %d\n", sim->diningRoom->dirtyKnives+sim->diningRoom->dirtyKnivesInWaiter);
        exit(EXIT_FAILURE);
    }
}

// Philosopher text & colors:

static char* philosopherStateText[] = {
    EMPTY_SYMBOL,     // P_BIRTH
    THINK_SYMBOL,     // P_THINKING
    HUNGRY_SYMBOL,    // P_HUNGRY
    EATING_SYMBOL,    // P_EATING
    FULL_SYMBOL,      // P_FULL
    DEAD_SYMBOL       // P_DEAD
};

static char* philosopherStateColor[] = {
    BLACK,            // P_BIRTH
    BLUE,             // P_THINKING
    RED,              // P_HUNGRY
    YELLOW,           // P_EATING
    GREEN,            // P_FULL
    BLUE              // P_DEAD
};

static char* philosopherMealText[] = {
    EMPTY_SYMBOL,     // P_NONE
    PIZZA_SYMBOL,     // P_GET_PIZZA
    PIZZA_SYMBOL,     // P_EAT_PIZZA
    SPAGHETTI_SYMBOL, // P_GET_SPAGHETTI
    SPAGHETTI_SYMBOL  // P_EAT_SPAGHETTI
};

static char* philosopherMealColor[] = {
    BLACK,            // P_NONE
    RED,              // P_GET_PIZZA
    GREEN,            // P_EAT_PIZZA
    RED,              // P_GET_SPAGHETTI
    GREEN             // P_EAT_SPAGHETTI
};

static char* philosopherCutleryText[] = {
    " ",              // P_NOTHING
    "f",              // P_GET_FORK
    "k",              // P_GET_KNIFE
    "f",              // P_FORK
    "k",              // P_KNIFE
    "f",              // P_PUT_FORK
    "k"               // P_PUT_KNIFE
};

static char* philosopherCutleryColor[] = {
    BLACK,            // P_NOTHING
    RED,              // P_GET_FORK
    RED,              // P_GET_KNIFE
    GREEN,            // P_FORK
    GREEN,            // P_KNIFE
    YELLOW,           // P_PUT_FORK
    YELLOW            // P_PUT_KNIFE
};

static void philosopherLogger(Philosopher* p)
{
    int i;

    printf("%s%s%s%s", BACKGROUND_WHITE, philosopherStateColor[p->state], philosopherStateText[p->state], END);
    printf("%s%s%s%s", BACKGROUND_WHITE, philosopherMealColor[p->meal], philosopherMealText[p->meal], END);
    for(i = 0; i < 2; i++)
        printf("%s%s%s%s", BACKGROUND_WHITE, philosopherCutleryColor[p->cutlery[i]], philosopherCutleryText[p->cutlery[i]], END);
}


// Waiter text & colors:

static char* waiterStateText[] = {
    EMPTY_SYMBOL,     // W_NONE
    WAIT_SYMBOL,      // W_SLEEP
    FULL_SYMBOL,      // W_REQUEST_CUTLERY
    FULL_SYMBOL,      // W_REQUEST_PIZZA
    FULL_SYMBOL,      // W_REQUEST_SPAGHETTI
    DEAD_SYMBOL       // W_DEAD
};

static char* waiterStateColor[] = {
    BLACK,            // W_NONE
    GREEN,            // W_SLEEP
    BLUE,             // W_REQUEST_CUTLERY
    BLUE,             // W_REQUEST_PIZZA
    BLUE,             // W_REQUEST_SPAGHETTI
    BLUE              // W_DEAD
};

static char* waiterRequestCutleryText[] = {
    "  ",             // W_INACTIVE
    "fk"              // W_ACTIVE
};

static char* waiterRequestCutleryColor[] = {
    BLACK,            // W_INACTIVE
    RED               // W_ACTIVE
};

static char* waiterRequestPizzaText[] = {
    EMPTY_SYMBOL,     // W_INACTIVE
    PIZZA_SYMBOL,     // W_ACTIVE
};

static char* waiterRequestPizzaColor[] = {
    BLACK,            // W_INACTIVE
    RED               // W_ACTIVE
};

static char* waiterRequestSpaghettiText[] = {
    EMPTY_SYMBOL,     // W_INACTIVE
    SPAGHETTI_SYMBOL  // W_ACTIVE
};

static char* waiterRequestSpaghettiColor[] = {
    BLACK,            // W_INACTIVE
    RED               // W_ACTIVE
};

static void waiterLogger(Waiter* w)
{
    printf("%s%s%s%s", BACKGROUND_WHITE, waiterStateColor[w->state], waiterStateText[w->state], END);
    printf("%s%s%s%s", BACKGROUND_WHITE, waiterRequestCutleryColor[w->reqCutlery], waiterRequestCutleryText[w->reqCutlery], END);
    printf("%s%s%s%s", BACKGROUND_WHITE, waiterRequestPizzaColor[w->reqPizza], waiterRequestPizzaText[w->reqPizza], END);
    printf("%s%s%s%s", BACKGROUND_WHITE, waiterRequestSpaghettiColor[w->reqSpaghetti], waiterRequestSpaghettiText[w->reqSpaghetti], END);
}

static void unsafe_logger(Simulation* sim)
{
    assert(sim != NULL);
    invariantCheck(sim);

    int i;

    int pizza = sim->diningRoom->pizza;
    int spaghetti = sim->diningRoom->spaghetti;
    int cleanForks = sim->diningRoom->cleanForks;
    int cleanKnives = sim->diningRoom->cleanKnives;
    int dirtyForks = sim->diningRoom->dirtyForks;
    int dirtyKnives = sim->diningRoom->dirtyKnives;
    int dirtyForksInWaiter = sim->diningRoom->dirtyForksInWaiter;
    int dirtyKnivesInWaiter = sim->diningRoom->dirtyKnivesInWaiter;

    if (showHeader)
    {
        printf("   Food   ");
        printf("   Forks   ");
        printf("   Knives  ");
        printf("\n");
        printf("  ");
        printf("       ");
        printf("%s%s%s", BLUE, " Clean", END);
        printf("%s%s%s", YELLOW," Dirty", END);
        printf("%s%s%s", BLUE, " Clean", END);
        printf("%s%s%s", YELLOW," Dirty", END);
        printf("%sWaiter:%s", EMPTY_SYMBOL, HEADER_SPACES);
        for(i = 0; i < sim->params->NUM_PHILOSOPHERS; i++)
            printf(" Ph%02d:%s", i+1, HEADER_SPACES);
        printf("\n");
        showHeader = 0;
    }
    printf("P:");
    printf("%s", pizza > 0 ? BLUE : RED);
    printf("%2d ",pizza);
    printf("%s", END);
    printf("S:");
    printf("%s", spaghetti > 0 ? BLUE : RED);
    printf("%2d ",spaghetti);
    printf("%s", END);
    printf("CF:");
    printf("%s", cleanForks > 0 ? BLUE : BLACK);
    printf("%2d ",cleanForks);
    printf("%s", END);
    printf("CK:");
    printf("%s", cleanKnives > 0 ? BLUE : BLACK);
    printf("%2d ",cleanKnives);
    printf("%s", END);
    printf("DF:");
    printf("%s", dirtyForks > 0 ? RED : BLACK);
    printf("%2d ",dirtyForks);
    printf("%s", END);
    printf("DK:");
    printf("%s", dirtyKnives > 0 ? RED : BLACK);
    printf("%2d ",dirtyKnives);
    printf("%s", END);
    printf("DFIW:");
    printf("%s",dirtyForksInWaiter > 0 ? RED : BLACK);
    printf("%2d ",dirtyForksInWaiter);
    printf("%s",END);
    printf("DKIW:");
    printf("%s",dirtyKnivesInWaiter > 0 ? RED : BLACK);
    printf("%2d ",dirtyKnivesInWaiter);
    printf("%s",END);
    printf("[");
    waiterLogger(sim->waiter);
    printf("]");
    for(i = 0; i < sim->params->NUM_PHILOSOPHERS; i++)
    {
        printf("[");
        philosopherLogger(sim->philosophers[i]);
        printf("]");
    }
    printf("\n");
}
