/**
 *  \brief Simulation static parameters data structure
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

typedef struct _Parameters_
{
    int NUM_PHILOSOPHERS;     // number of philosophers
    int PHILOSOPHER_MIN_LIVE; // minimum number of iterations of philosophers life cycle
    int PHILOSOPHER_MAX_LIVE; // maximum number of iterations of philosophers life cycle
    int NUM_FORKS;            // number of forks
    int NUM_KNIVES;           // number of knives
    int NUM_PIZZA;            // number of pizza meals in each replenish operation
    int NUM_SPAGHETTI;        // number of spaghetti meals in each replenish operation
    int THINK_TIME;           // maximum milliseconds for thinking (the actual time should be a random value in interval [0;THINK_TIME])
    int CHOOSE_PIZZA_PROB;    // probability to choose a pizza meal against a spaghetti meal
    int EAT_TIME;             // maximum milliseconds for eating (the actual time should be a random value in interval [0;EAT_TIME])
    int WASH_TIME;            // maximum milliseconds for washing (the actual time should be a random value in interval [0;WASH_TIME])
} Parameters;

#endif
