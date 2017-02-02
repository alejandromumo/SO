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
#include "logger.h"
#include "simulation.h"
#include "waiter.h"
#include <pthread.h> /* added */

/* put your code here */

extern Simulation* sim;

static int req = 0; /* request flag */
static int kill = 0;

static pthread_mutex_t REQUEST_ACCESS = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t request_available = PTHREAD_COND_INITIALIZER;


void waiter_lifecycle(){
	while(1) {
		sim->waiter->state = W_SLEEP; /* Sleep by default when not attending a request */

		pthread_mutex_lock(&REQUEST_ACCESS);
		while(req == 0)
			pthread_cond_wait(&request_available, &REQUEST_ACCESS);
		//printf("Waiter will make the request\n");
		if(req == 1){
			request_pizza();
		}else if(req == 2){
			request_spaghetti();
		}else if(req == 3){
			request_washed_cutlery();
		}else if(req == 4){
			request_washed_cutlery();
			kill = 1;
		}
		//printf("Waiter has done the request\n");
		req = 0;
		pthread_mutex_unlock(&REQUEST_ACCESS);
		if(kill == 1)
			break;
		logger(sim);
	}
	kill_waiter();
}

int trylock(){
	/* Avoid deadlock:
		- If REQUEST_ACCESS is being used or REQUEST_ACCESS isn't being used but there is a peding request, it means the waiter can't accept a new request and it will unlock DININGROOM_ACCESS (because the waiter may need it) and try again
	*/
	int value;
	if(pthread_mutex_trylock(&REQUEST_ACCESS) == 0){
		value = req;
		pthread_mutex_unlock(&REQUEST_ACCESS);
		return value;
	}
	return 1;
}

void make_request(int n){
	pthread_mutex_lock(&REQUEST_ACCESS);
	req = n;
	//printf("Waiter has a new request\n");
	pthread_cond_signal(&request_available);
	pthread_mutex_unlock(&REQUEST_ACCESS);
}

void request_pizza(){
	sim->waiter->state = W_REQUEST_PIZZA;
	sim->waiter->reqPizza = W_ACTIVE;
	logger(sim);

	replenish_pizza();

	sim->waiter->state = W_SLEEP;
	sim->waiter->reqPizza = W_INACTIVE;
}

void request_spaghetti(){
	sim->waiter->state = W_REQUEST_SPAGHETTI;
	sim->waiter->reqSpaghetti = W_ACTIVE;
	logger(sim);

	replenish_spaghetti();

	sim->waiter->state = W_SLEEP;
	sim->waiter->reqSpaghetti = W_INACTIVE;
}

void request_washed_cutlery(){
	sim->waiter->state = W_REQUEST_CUTLERY;
	sim->waiter->reqCutlery = W_ACTIVE;
	logger(sim);

	replenish_cutlery();

	sim->waiter->state = W_SLEEP;
	sim->waiter->reqCutlery = W_INACTIVE;
}

void kill_waiter(){
	sim->waiter->state = W_DEAD;
	logger(sim);
}
