/**
 * \brief Simulation data structure
 *
 * \author Miguel Oliveira e Silva - 2016
 */

#ifndef SIMULATION_H
#define SIMULATION_H

struct _Waiter_;
struct _Parameters_;
struct _DiningRoom_;
struct _Philosopher_;

typedef struct _Simulation_ {
    struct _Parameters_* params;
    struct _DiningRoom_* diningRoom;
    struct _Philosopher_** philosophers;
    struct _Waiter_* waiter;
} Simulation;

#include "parameters.h"
#include "dining-room.h"
#include "philosopher.h"
#include "waiter.h"

Simulation *simulation_new(Parameters *params);
Simulation *simulation_share(Simulation *s);
void simulation_unshare(Simulation *s);

bool simulation_are_philosophers_dead();

void simulation_start(Simulation *s, pid_t *pid);
void simulation_stop(Simulation *s, pid_t *pid);

#endif
