/**
 *  \brief Philosopher data structures
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef PHILOSOPHER_H
#define PHILOSOPHER_H

#include <stdbool.h>
#include "simulation.h"

typedef enum {
    P_BIRTH,         // philosopher birth (initial state)
    P_THINKING,      // philosopher is thinking
    P_HUNGRY,        // philosopher is hungry
    P_EATING,        // philosopher is eating
    P_FULL,          // philosopher has eaten and is full
    P_DEAD           // philosopher dead
} PhilosopherState;

typedef enum {
    P_NONE,          // philosopher activity has no relation to meals
    P_GET_PIZZA,     // philosopher wants pizza
    P_EAT_PIZZA,     // philosopher is eating pizza
    P_GET_SPAGHETTI, // philosopher wants spaghetti
    P_EAT_SPAGHETTI  // philosopher is eating spaghetti
} PhilosopherMealState;

typedef enum {
    P_NOTHING,       // philosopher activity has no relation to cutlery
    P_GET_FORK,      // philosopher needs a clean fork
    P_GET_KNIFE,     // philosopher needs a clean knife
    P_FORK,          // philosopher has a fork
    P_KNIFE,         // philosopher has a knife
    P_PUT_FORK,      // philosopher is returning a dirty fork
    P_PUT_KNIFE      // philosopher is returning a dirty knife
} PhilosopherCutleryState;

typedef struct _Philosopher_ {
    PhilosopherState state;             // current philosopher state
    PhilosopherMealState meal;          // current philosopher meal state
    PhilosopherCutleryState cutlery[2]; // current philosopher cutlery state
} Philosopher;


Philosopher *philosopher_new();
void philosopher_lifecycle(Philosopher *p, Simulation *s);
void philosopher_think(Philosopher *p, Simulation *s);
void philosopher_hungry(Philosopher *p, Simulation *s);
void philosopher_eat(Philosopher *p, Simulation *s);
void philosopher_kill(Philosopher *p, Simulation *s);

#endif
