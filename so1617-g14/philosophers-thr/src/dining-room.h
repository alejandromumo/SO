/**
 * \brief Dining room data structures
 *  
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef DINING_ROOM_H
#define DINING_ROOM_H

#include "simulation.h"

typedef struct _DiningRoom_ {
   int pizza;                  // number of pizza meals available in dining room [0;NUM_PIZZA]
   int spaghetti;              // number of spaghetti meals available in dining room [0;NUM_SPAGHETTI]
   int cleanForks;             // number of clean forks available in dining room [0;NUM_FORKS]
   int cleanKnives;            // number of clean knives available in dining room [0;NUM_KNIVES]
   int dirtyForks;             // number of dirty forks in dining room [0;NUM_FORKS]
   int dirtyKnives;            // number of dirty knives in dining room [0;NUM_KNIVES]
   int dirtyForksInWaiter;     // number of dirty forks in waiter (i.e. the dirty forks that are being washed)
   int dirtyKnivesInWaiter;    // number of dirty knives in waiter (i.e. the dirty knives that are being washed)
   int philosophersAlive;      // number of philosophers alive
} DiningRoom;


/* put your code here */

void get_spaghetti(int id);

void get_pizza(int id);

void get_two_forks(int id);

void get_fork_knife(int id);

void drop_two_forks(int id);

void drop_fork_knife(int id);

void replenish_pizza();

void replenish_spaghetti();

void replenish_cutlery();

void kill_philosopher(int id);

#endif

