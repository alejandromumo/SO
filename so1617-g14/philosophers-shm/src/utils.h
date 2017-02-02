/**
 *  \brief Utility functions and data structures
 *
 * \author Fábio Maia
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <semaphore.h>

void *memory_allocate(size_t size);
void process_fork(pid_t *pidp, void (*routine)(void*, void*), void *arg1, void *arg2);
int rand_interval(int min, int max);
int msleep(int t);

sem_t *semaphore_open(const char *name, unsigned int value);
void semaphore_wait(sem_t *sem);
void semaphore_post(sem_t *sem);
void semaphore_close(sem_t *sem);
void semaphore_unlink(const char *name);
int semaphore_getvalue(sem_t *sem);

#endif
