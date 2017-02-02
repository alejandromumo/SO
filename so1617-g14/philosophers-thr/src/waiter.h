/**
 *  \brief Waiter data structures
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef WAITER_H
#define WAITER_H

typedef enum {
	W_NONE,                           // waiter initial state
	W_SLEEP,                          // waiter sleeping (waiting for requests)
	W_REQUEST_CUTLERY,                // waiter processing a request for clean cutlery
	W_REQUEST_PIZZA,                  // waiter processing a request for pizza meals
	W_REQUEST_SPAGHETTI,              // waiter processing a request for spaghetti meals
	W_DEAD                            // waiter is dead
} WaiterState;

typedef enum {
	W_INACTIVE,                       // no pending request
	W_ACTIVE                          // a pending request is active
} WaiterPendingState;

typedef struct _Waiter_ {
	WaiterState state;                // current waiter state
	WaiterPendingState reqCutlery;    // waiter's request cutlery state
	WaiterPendingState reqPizza;      // waiter's request pizza state
	WaiterPendingState reqSpaghetti;  // waiter's request spaghetti state
} Waiter;


/* put your code here */
void waiter_lifecycle();
int trylock();
void make_request(int n);
void request_pizza();
void request_spaghetti();
void request_washed_cutlery();
void kill_waiter();
#endif
