/**
 *  \brief Waiter module
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "dining-room.h"
#include "simulation.h"
#include "waiter.h"
#include "utils.h"
#include "logger.h"

Waiter *waiter_new()
{
    Waiter *w = memory_allocate(sizeof(Waiter));
    w->state = W_NONE;
    w->reqCutlery = W_INACTIVE;
    w->reqPizza = W_INACTIVE;
    w->reqSpaghetti = W_INACTIVE;
    return w;
}

void waiter_lifecycle(Waiter *w, Simulation *s)
{
    sem_t *signal_waiter = semaphore_open("/signal_waiter", 0);
    while (!simulation_are_philosophers_dead(s))
    {
        w->state = W_SLEEP; // Sleep by default when not attending a request
        logger(s);
        semaphore_wait(signal_waiter);

        waiter_replenish_pizza(s->waiter, s);
        waiter_replenish_spaghetti(s->waiter, s);
        waiter_wash_cutlery(s->waiter, s);

        semaphore_post(signal_waiter);
    }

    // Leave the dining room clean after death
    waiter_wash_cutlery(w, s);
    w->state = W_DEAD;
    logger(s);
}

void waiter_replenish_pizza(Waiter *w, Simulation *s)
{
    sem_t *signal_replenish_pizza = semaphore_open("/signal_replenish_pizza", 0);


    if (s->waiter->reqPizza == W_ACTIVE)
    {
        s->waiter->state = W_REQUEST_PIZZA;

        // Replenish pizza
        dining_room_replenish_pizza(s);

        // No longer active
        s->waiter->reqPizza = W_INACTIVE;

        logger(s);

        // Signal that pizza has been replenished
        semaphore_post(signal_replenish_pizza);

    }
}

void waiter_replenish_spaghetti(Waiter *w, Simulation *s)
{
    sem_t *signal_replenish_spaghetti = semaphore_open("/signal_replenish_spaghetti", 0);


    if (s->waiter->reqSpaghetti == W_ACTIVE)
    {
        s->waiter->state = W_REQUEST_PIZZA;

        // Replenish spaghetti
        dining_room_replenish_spaghetti(s);

        // No longer active
        s->waiter->reqSpaghetti = W_INACTIVE;

        logger(s);

        // Signal that spaghetti has been replenished
        semaphore_post(signal_replenish_spaghetti);

    }
}

void waiter_wash_cutlery(Waiter *w, Simulation *s)
{
    sem_t *signal_wash_cutlery = semaphore_open("/signal_wash_cutlery", 0);

    if (s->waiter->reqCutlery == W_ACTIVE)
    {
        s->waiter->state = W_REQUEST_CUTLERY;

        // Wash cutlery
        dining_room_prepare_wash(s);
        msleep(s->params->WASH_TIME);
        dining_room_wash_cutlery(s);

        // No longer active
        s->waiter->reqCutlery = W_INACTIVE;

        logger(s);

        // Signal that cutlery has been washed
        semaphore_post(signal_wash_cutlery);
    }
}
