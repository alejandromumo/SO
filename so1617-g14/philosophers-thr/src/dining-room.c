/**
 * \brief Dining room
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <locale.h>
#include "parameters.h"
#include "dining-room.h"
#include "waiter.h"
#include "logger.h"
#include <pthread.h> /* added */

/* put your code here */

extern Simulation * sim;

static pthread_mutex_t DININGROOM_ACCESS = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t pizza_available = PTHREAD_COND_INITIALIZER, spaghetti_available = PTHREAD_COND_INITIALIZER, cleancutlery_available = PTHREAD_COND_INITIALIZER;


void get_spaghetti(int id){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	while(sim->diningRoom->spaghetti == 0){
		/* philosopher tries to make a request */
		if(trylock() == 0)
			make_request(2);
		else{
			pthread_mutex_unlock(&DININGROOM_ACCESS);
			return get_spaghetti(id);
		}

		/* philosopher waits... */
		pthread_cond_wait(&spaghetti_available, &DININGROOM_ACCESS);
	}

	sim->diningRoom->spaghetti -= 1;

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void get_pizza(int id){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	while(sim->diningRoom->pizza == 0){
		/* philosopher tries to make a request */
		if(trylock() == 0)
			make_request(1);
		else{
			pthread_mutex_unlock(&DININGROOM_ACCESS);
			return get_pizza(id);
		}

		/* philosopher waits... */
		pthread_cond_wait(&pizza_available, &DININGROOM_ACCESS);
	}

	sim->diningRoom->pizza -= 1;

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void get_two_forks(int id){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	while(sim->diningRoom->cleanForks < 2){
		/* philosopher tries to make a request */
		if(trylock() == 0)
			make_request(3);
		else{
			pthread_mutex_unlock(&DININGROOM_ACCESS);
			return get_two_forks(id);
		}

		/* philosopher waits... */
		pthread_cond_wait(&cleancutlery_available, &DININGROOM_ACCESS);
	}

	sim->diningRoom->cleanForks -= 2;

	/* update philosopher info */
	sim->philosophers[id]->cutlery[0] = P_FORK;
	sim->philosophers[id]->cutlery[1] = P_FORK;

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void get_fork_knife(int id){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	while(sim->diningRoom->cleanForks == 0 || sim->diningRoom->cleanKnives == 0){
		/* philosopher tries to make a request */
		if(trylock() == 0)
			make_request(3);
		else{
			pthread_mutex_unlock(&DININGROOM_ACCESS);
			return get_fork_knife(id);
		}

		/* philosopher waits... */
		pthread_cond_wait(&cleancutlery_available, &DININGROOM_ACCESS);
	}

	sim->diningRoom->cleanForks -= 1;
	sim->diningRoom->cleanKnives -= 1;

	/* update philosopher info */
	sim->philosophers[id]->cutlery[0] = P_FORK;
	sim->philosophers[id]->cutlery[1] = P_KNIFE;

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void drop_two_forks(int id){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	sim->diningRoom->dirtyForks += 2;

	/* update philosopher info */
	sim->philosophers[id]->cutlery[0] = P_PUT_FORK;
	sim->philosophers[id]->cutlery[1] = P_PUT_FORK;

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void drop_fork_knife(int id){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	sim->diningRoom->dirtyForks += 1;
	sim->diningRoom->dirtyKnives += 1;

	/* update philosopher info */
	sim->philosophers[id]->cutlery[0] = P_PUT_FORK;
	sim->philosophers[id]->cutlery[1] = P_PUT_KNIFE;

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void replenish_pizza(){
	pthread_mutex_lock(&DININGROOM_ACCESS);

  	sim->diningRoom->pizza = sim->params->NUM_PIZZA;

  	/* Signal to philosopher to stop waiting... */
  	pthread_cond_signal(&pizza_available);

  	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void replenish_spaghetti(){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	sim->diningRoom->spaghetti = sim->params->NUM_SPAGHETTI;

	/* Signal to philosopher to stop waiting... */
  	pthread_cond_signal(&spaghetti_available);

  	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void replenish_cutlery(){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	/* dirty forks and knives in waiter */
	sim->diningRoom->dirtyKnivesInWaiter = sim->diningRoom->dirtyKnives;
	sim->diningRoom->dirtyForksInWaiter = sim->diningRoom->dirtyForks;

	sim->diningRoom->dirtyForks = 0;
	sim->diningRoom->dirtyKnives = 0;

	logger(sim);

	usleep(sim->params->WASH_TIME*1000);

	/* forks and knives are clean */
	sim->diningRoom->cleanKnives +=  sim->diningRoom->dirtyKnivesInWaiter;
	sim->diningRoom->cleanForks += sim->diningRoom->dirtyForksInWaiter;

	sim->diningRoom->dirtyForksInWaiter = 0;
	sim->diningRoom->dirtyKnivesInWaiter = 0;

	/* Signal to philosopher to stop waiting... */
  	pthread_cond_signal(&cleancutlery_available);

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}

void kill_philosopher(int id){
	pthread_mutex_lock(&DININGROOM_ACCESS);

	sim->diningRoom->philosophersAlive--;
	//printf("Philosopher %d is dead (%d left)\n", id, sim->diningRoom->philosophersAlive);

	if(sim->diningRoom->philosophersAlive == 0){
		if(sim->diningRoom->cleanForks != sim->params->NUM_FORKS || sim->diningRoom->cleanKnives != sim->params->NUM_KNIVES){
			//printf("All philosophers are dead\n");
			make_request(4);

			while(sim->diningRoom->cleanForks != sim->params->NUM_FORKS || sim->diningRoom->cleanKnives != sim->params->NUM_KNIVES)
				pthread_cond_wait(&cleancutlery_available, &DININGROOM_ACCESS);
		}
	}

	pthread_mutex_unlock(&DININGROOM_ACCESS);
}
