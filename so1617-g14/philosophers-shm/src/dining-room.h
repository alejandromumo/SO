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
} DiningRoom;

DiningRoom *dining_room_new(int pizza, int spaghetti, int cleanForks, int cleanKnives, int dirtyForks, int dirtyKnives, int dirtyForksInWaiter, int dirtyKnivesInWaiter);

void dining_room_fetch_pizza(Simulation *s);
void dining_room_fetch_spaghetti(Simulation *s);

void dining_room_replenish_pizza(Simulation *s);
void dining_room_replenish_spaghetti(Simulation *s);

void dining_room_fetch_clean_fork(Simulation *s);
void dining_room_fetch_clean_knife(Simulation *s);

void dining_room_return_dirty_fork(Simulation *s);
void dining_room_return_dirty_knife(Simulation *s);

void dining_room_prepare_wash(Simulation *s);
void dining_room_wash_cutlery(Simulation *s);

#endif
