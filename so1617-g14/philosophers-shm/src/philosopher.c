/**
 *  \brief Philosopher module
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#include "philosopher.h"
#include "dining-room.h"
#include "simulation.h"
#include "logger.h"
#include "utils.h"

Philosopher *philosopher_new()
{
    Philosopher *p = memory_allocate(sizeof(Philosopher));
    p->state = P_BIRTH;
    p->meal = P_NONE;
    p->cutlery[0] = P_NOTHING;
    p->cutlery[1] = P_NOTHING;
    return p;
}

void philosopher_lifecycle(Philosopher *p, Simulation *s)
{
    int lifetime = rand_interval(s->params->PHILOSOPHER_MIN_LIVE, s->params->PHILOSOPHER_MAX_LIVE);
    int i;
    for(i = 0; i < lifetime; i++)
    {
        philosopher_think(p, s);
        philosopher_hungry(p, s);
        philosopher_eat(p, s);
    }

    philosopher_kill(p, s);
}

void philosopher_think(Philosopher *p, Simulation *s)
{
    p->state = P_THINKING;
    logger(s);
    msleep(rand_interval(0, s->params->THINK_TIME));
}

void philosopher_hungry(Philosopher *p, Simulation *s)
{
    p->state = P_HUNGRY;

    if (rand_interval(0, 100) <= s->params->CHOOSE_PIZZA_PROB)
    {
        p->meal = P_GET_PIZZA;
        dining_room_fetch_pizza(s);

        p->cutlery[0] = P_GET_FORK;
        p->cutlery[1] = P_GET_KNIFE;
        dining_room_fetch_pizza_cutlery(s);
    }
    else
    {
        p->meal = P_GET_SPAGHETTI;
        dining_room_fetch_spaghetti(s);

        p->cutlery[0] = P_GET_FORK;
        p->cutlery[1] = P_GET_FORK;
        dining_room_fetch_spaghetti_cutlery(s);

    }

    logger(s);
}

void philosopher_eat(Philosopher *p, Simulation *s)
{
    p->state = P_EATING;

    // Eat chosen meal
    if (p->meal == P_GET_PIZZA)
    {
        p->meal = P_EAT_PIZZA;
        p->cutlery[0] = P_FORK;
        p->cutlery[1] = P_KNIFE;

    }
    else
    {
        p->meal = P_EAT_SPAGHETTI;
        p->cutlery[0] = P_FORK;
        p->cutlery[1] = P_FORK;
    }

    logger(s);

    // Pretend to eat
    msleep(rand_interval(0, s->params->EAT_TIME));

    // Return dirty cutlery
    if (p->meal == P_EAT_PIZZA)
    {
        printf("Returning pizza cutlery\n");
        dining_room_return_pizza_cutlery(s);
        printf("Returned pizza cutlery successfully\n");
    }
    else
    {
        printf("Returning pizza cutlery\n");
        dining_room_return_spaghetti_cutlery(s);
        printf("Returned spaghetti cutlery successfully\n");

    }

    p->state = P_FULL;
    p->meal = P_NONE;
    p->cutlery[0] = P_NOTHING;
    p->cutlery[1] = P_NOTHING;

    logger(s);
}

void philosopher_kill(Philosopher *p, Simulation *s)
{
    // Kill the philosopher
    p->state = P_DEAD;
    logger(s);
}
