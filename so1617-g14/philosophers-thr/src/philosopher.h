/**
 *  \brief Philosopher data structures
 *  
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef PHILOSOPHER_H
#define PHILOSOPHER_H

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


/* put your code here */

void philosopher_lifecycle(int id);

#endif
