/**
 * \brief Dining room
 *
 * \author Miguel Oliveira e Silva - 2016
 */


#include "semaphore.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>

#include "utils.h"
#include "dining-room.h"
#include "waiter.h"
#include "philosopher.h"
#include "logger.h"
#include "simulation.h"

DiningRoom *dining_room_new(int pizza, int spaghetti, int cleanForks, int cleanKnives, int dirtyForks, int dirtyKnives, int dirtyForksInWaiter, int dirtyKnivesInWaiter)
{
    DiningRoom *d = memory_allocate(sizeof(DiningRoom));
    d->pizza = pizza;
    d->spaghetti = spaghetti;
    d->cleanForks = cleanForks;
    d->cleanKnives = cleanKnives;
    d->dirtyForks = dirtyForks;
    d->dirtyKnives = dirtyKnives;
    d->dirtyForksInWaiter = dirtyForksInWaiter;
    d->dirtyKnivesInWaiter = dirtyKnivesInWaiter;
    return d;
}

/**
 * Fetch pizza
 */
void dining_room_fetch_pizza(Simulation *s)
{
    sem_t *mutex_pizza = semaphore_open("/mutex_pizza", 1);
    sem_t *mutex_req_pizza = semaphore_open("/mutex_req_pizza", 1);

    sem_t *signal_replenish_pizza = semaphore_open("/signal_replenish_pizza", 0);
    sem_t *signal_waiter = semaphore_open("/signal_waiter", 0);


    semaphore_wait(mutex_pizza);
    semaphore_wait(mutex_req_pizza);
    int piz = s->diningRoom->pizza;

    if (piz == 0)
    {
        // Set pizza request
        s->waiter->reqPizza = W_ACTIVE;

        // Wake up waiter
        semaphore_post(signal_waiter);

        // Wait for pizza to be replenished
        semaphore_wait(signal_replenish_pizza);
    }

    s->diningRoom->pizza--;
    semaphore_post(mutex_pizza);
    semaphore_post(mutex_req_pizza);
}

/**
 * Fetch spaghetti
 */
 void dining_room_fetch_spaghetti(Simulation *s)
 {
     sem_t *mutex_spaghetti = semaphore_open("/mutex_spaghetti", 1);
     sem_t *mutex_req_spaghetti = semaphore_open("/mutex_req_spaghetti", 1);

     sem_t *signal_replenish_spaghetti = semaphore_open("/signal_replenish_spaghetti", 0);
     sem_t *signal_waiter = semaphore_open("/signal_waiter", 0);


     semaphore_wait(mutex_spaghetti);
     semaphore_wait(mutex_req_spaghetti);
     int spag = s->diningRoom->spaghetti;

     if (spag == 0)
     {
         // Set spaghetti request
         s->waiter->reqSpaghetti = W_ACTIVE;

         // Wake up waiter
         semaphore_post(signal_waiter);

         // Wait for spaghetti to be replenished
         semaphore_wait(signal_replenish_spaghetti);
     }

     s->diningRoom->spaghetti--;
     semaphore_post(mutex_spaghetti);
     semaphore_post(mutex_req_spaghetti);
 }


/**
 * Fetch cutlery for pizza
 */
void dining_room_fetch_pizza_cutlery(Simulation *s)
{
  sem_t *mutex_clean_forks = semaphore_open("/mutex_clean_forks", 1);
  sem_t *mutex_clean_knives = semaphore_open("/mutex_clean_knives", 1);
  sem_t *mutex_req_cutlery = semaphore_open("/mutex_req_cutlery", 1);
  sem_t *mutex_dirty_forks = semaphore_open("/mutex_dirty_forks",1);
  sem_t *mutex_dirty_knives = semaphore_open("/mutex_dirty_knives", 1);


  sem_t *signal_wash_cutlery = semaphore_open("/signal_wash_cutlery", 0);
  sem_t *signal_waiter = semaphore_open("/signal_waiter", 0);


  // Entering critical zone

  semaphore_wait(mutex_clean_forks);
  semaphore_wait(mutex_dirty_forks);
  semaphore_wait(mutex_clean_knives);
  semaphore_wait(mutex_dirty_knives);
  semaphore_wait(mutex_req_cutlery);

  // TODO equacionar condição para não lavar desnecessariamente loiça
  if((s->diningRoom->cleanForks == 0 && s->diningRoom->dirtyForks > 0) || (s->diningRoom->cleanKnives == 0 && s->diningRoom->dirtyKnives > 0))
  {
    // Set cutlery request
    s->waiter->reqCutlery = W_ACTIVE;

    // Wake up waiter
    semaphore_post(signal_waiter);

    printf("Waiting for clean forks...\n");

    // Wait for cutlery to be washed
    semaphore_wait(signal_wash_cutlery);
  }
  if(s->diningRoom->cleanForks > 0 && s->diningRoom->cleanKnives > 0)
  {
    // Philosopher gets his desired cutlery
    s->diningRoom->cleanForks--;
    s->diningRoom->cleanKnives--;
  }

  // Leaving critical zone
  semaphore_post(mutex_clean_forks);
  semaphore_post(mutex_dirty_forks);
  semaphore_post(mutex_clean_knives);
  semaphore_post(mutex_dirty_knives);
  semaphore_post(mutex_req_cutlery);

}

/**
 * Fetch cutlery for spaghetti
 */
void dining_room_fetch_spaghetti_cutlery(Simulation *s)
{
  sem_t *mutex_clean_forks = semaphore_open("/mutex_clean_forks", 1);

  sem_t *mutex_req_cutlery = semaphore_open("/mutex_req_cutlery", 1);
  sem_t *mutex_dirty_forks = semaphore_open("/mutex_dirty_forks",1);


  sem_t *signal_wash_cutlery = semaphore_open("/signal_wash_cutlery", 0);
  sem_t *signal_waiter = semaphore_open("/signal_waiter", 0);


  // Entering critical zone
  semaphore_wait(mutex_clean_forks);
  semaphore_wait(mutex_dirty_forks);
  semaphore_wait(mutex_req_cutlery);

  if(s->diningRoom->dirtyForks + s->diningRoom->cleanForks >= 2)
  {
    // Set cutlery request
    s->waiter->reqCutlery = W_ACTIVE;

    // Wake up waiter
    semaphore_post(signal_waiter);

    printf("Waiting for clean forks...\n");

    // Wait for cutlery to be washed
    semaphore_wait(signal_wash_cutlery);
  }
  if(s->diningRoom->cleanForks >= 2)
  {
    // Philosopher gets his desired cutlery
    s->diningRoom->cleanForks--;
    s->diningRoom->cleanForks--;
  }
  // Leaving critical zone
  semaphore_post(mutex_clean_forks);
  semaphore_post(mutex_dirty_forks);
  semaphore_post(mutex_req_cutlery);

}

/**
 * Replenish pizza when there isn't any
 */
void dining_room_replenish_pizza(Simulation *s)
{
    printf("Replenishing pizza...\n");
    s->diningRoom->pizza = s->params->NUM_PIZZA;
}

/**
 * Replenish spaghetti when there isn't any
 */
void dining_room_replenish_spaghetti(Simulation *s)
{
    printf("Replenishing spaghetti...\n");
    s->diningRoom->spaghetti = s->params->NUM_SPAGHETTI;
}

/**
 * Moves the dirty cutlery used by philosophers to the waiter
 */
void dining_room_prepare_wash(Simulation *s)
{
    // Prepare forks
    s->diningRoom->dirtyForksInWaiter = s->diningRoom->dirtyForks;
    s->diningRoom->dirtyForks = 0;

    // Prepare knives
    s->diningRoom->dirtyKnivesInWaiter = s->diningRoom->dirtyKnives;
    s->diningRoom->dirtyKnives = 0;
}

/**
 * Washes the dirty cutlery used by philosophers
 */
void dining_room_wash_cutlery(Simulation *s)
{
    // Wash forks
    printf("Washing %d dirty forks in waiter...\n", s->diningRoom->dirtyForksInWaiter);
    s->diningRoom->cleanForks += s->diningRoom->dirtyForksInWaiter;

    // Wash knives
    printf("Washing %d dirty knives in waiter...\n", s->diningRoom->dirtyKnivesInWaiter);
    s->diningRoom->cleanKnives += s->diningRoom->dirtyKnivesInWaiter;

    s->diningRoom->dirtyForksInWaiter = 0;
    s->diningRoom->dirtyKnivesInWaiter = 0;
}

void dining_room_return_spaghetti_cutlery(Simulation *s)
{
    sem_t *mutex_dirty_forks = semaphore_open("/mutex_dirty_forks", 1);
    sem_t *mutex_clean_forks = semaphore_open("/mutex_clean_forks", 1);

    semaphore_wait(mutex_dirty_forks);
    semaphore_wait(mutex_clean_forks);

    /* TODO Confirmar se estas condições são realmente necessárias
       ou é um erro nosso o número de knives/forks ser maior que o inicial
    */
    if(s->diningRoom->dirtyForks + s->diningRoom->cleanForks + s->diningRoom->dirtyForksInWaiter < s->params->NUM_FORKS-1)
    {
      s->diningRoom->dirtyForks++;
      s->diningRoom->dirtyForks++;

    }
    semaphore_post(mutex_dirty_forks);
    semaphore_post(mutex_clean_forks);
    printf("Returning dirty forks...\n");
}

void dining_room_return_pizza_cutlery(Simulation *s)
{
    sem_t *mutex_dirty_forks = semaphore_open("/mutex_dirty_forks", 1);
    sem_t *mutex_clean_forks = semaphore_open("/mutex_clean_forks", 1);
    sem_t *mutex_dirty_knives = semaphore_open("/mutex_dirty_knives", 1);
    sem_t *mutex_clean_knives = semaphore_open("/mutex_clean_knives", 1);

    // Entering critical zone
    semaphore_wait(mutex_dirty_knives);
    semaphore_wait(mutex_clean_knives);
    semaphore_wait(mutex_dirty_forks);
    semaphore_wait(mutex_clean_forks);

    /* TODO Confirmar se estas condições são realmente necessárias
       ou é um erro nosso o número de knives/forks ser maior que o inicial
    */
    if(s->diningRoom->dirtyKnives + s->diningRoom->cleanKnives + s->diningRoom->dirtyKnivesInWaiter < s->params->NUM_KNIVES)
    {
      s->diningRoom->dirtyKnives++;
    }
    if(s->diningRoom->dirtyForks + s->diningRoom->cleanForks + s->diningRoom->dirtyForksInWaiter < s->params->NUM_FORKS)
    {
      s->diningRoom->dirtyForks++;
    }

    // Leaving critical zone
    semaphore_post(mutex_dirty_knives);
    semaphore_post(mutex_clean_knives);
    semaphore_post(mutex_dirty_forks);
    semaphore_post(mutex_clean_forks);

    printf("Returning dirty knives...\n");
}
