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
#include "philosopher.h"
#include "dining-room.h"
#include "simulation.h"
#include "logger.h"

/* put your code here */

extern Simulation* sim;


void philosopher_lifecycle(int id){
	int st;
	int life_time, meal;
	int i = 0;

	/* Get random life time based on the given parameters */
	life_time = (rand() % (sim->params->PHILOSOPHER_MAX_LIVE - sim->params->PHILOSOPHER_MIN_LIVE)) + sim->params->PHILOSOPHER_MIN_LIVE;
	//printf("Philosopher %d living for %d iterations\n", id, life_time);

	while(life_time > i){
		/* Think for a random time */
		st = rand() % sim->params->THINK_TIME;
		sim->philosophers[id]->state = P_THINKING; /* Philosopher is thinking */
		//printf("Philosopher %d thinking for %d milliseconds\n", id, st);
		logger(sim);
		usleep(st * 1000); /* millisecond to microseconds */

		/* Choose meal */
		meal = rand() % 100;
		sim->philosophers[id]->state = P_HUNGRY; /* Philosopher is hungry */
		if(meal <= sim->params->CHOOSE_PIZZA_PROB){
			//printf("Philosopher %d is hungry and has chosen pizza\n", id);
			/* Philosopher wants pizza */
			sim->philosophers[id]->meal = P_GET_PIZZA;
			logger(sim);
			/* Fetch meal */
			get_pizza(id);
			sim->philosophers[id]->meal = P_EAT_PIZZA;
			/* Fetch cutlery */
			//printf("Philosopher %d is getting a fork and a knife\n", id);
			sim->philosophers[id]->cutlery[0] = P_GET_FORK; /* Philosopher needs a fork */
			sim->philosophers[id]->cutlery[1] = P_GET_KNIFE; /* Philosopher needs a knife */
			logger(sim);
			get_fork_knife(id);
		}else{
			//printf("Philosopher %d is hungry and has chosen spaghetti\n", id);
			/* Philosopher wants spaghetti */
			sim->philosophers[id]->meal = P_GET_SPAGHETTI;
			logger(sim);
			/* Fetch meal */
			get_spaghetti(id);
			sim->philosophers[id]->meal = P_EAT_SPAGHETTI;
			/* Fetch cutlery */
			//printf("Philosopher %d is getting two forks\n", id);
			sim->philosophers[id]->cutlery[0] = P_GET_FORK; /* Philosopher needs a fork */
			sim->philosophers[id]->cutlery[1] = P_GET_FORK; /* Philosopher needs a fork */
			logger(sim);
			get_two_forks(id);
		}
		logger(sim);

		/* Eat for a random time */
		st = rand() % sim->params->EAT_TIME;
		sim->philosophers[id]->state = P_EATING; /* Philosopher is eating */
		//printf("Philosopher %d eating for %d milliseconds\n", id, st);
		logger(sim);
		usleep(st * 1000); /* millisecond to microseconds */

		/* Return dirty cutlery */
		if(meal <= sim->params->CHOOSE_PIZZA_PROB){
			drop_fork_knife(id);
		}else{
			drop_two_forks(id);
		}
		logger(sim);

		//printf("Philosopher %d returned the cutlery\n", id);
		/* No relation with cutlery */
		sim->philosophers[id]->cutlery[0] = P_NOTHING;
		sim->philosophers[id]->cutlery[1] = P_NOTHING;

		sim->philosophers[id]->state = P_FULL; /* Philosopher has eaten and is full */
		//printf("Philosopher %d has eaten and is full\n", id);

		/* No relation with meals */
		sim->philosophers[id]->meal = P_NONE;
		logger(sim);
		/* Increment counter */
		i++;
	}

	/* Kill philosopher */
	sim->philosophers[id]->state = P_DEAD; /* Philosopher is dead */
	logger(sim);
	kill_philosopher(id);
}
