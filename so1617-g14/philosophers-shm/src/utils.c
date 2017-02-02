/**
*  \brief Utility functions and data structures
*
* \author Fábio Maia
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

/**
 * Allocate memory responsibly
 */
void *memory_allocate(size_t size)
{
    void *result = malloc(size);

    if (!result)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    return result;
}

/**
 * Spawn a new process and run a given routine on the child process
 */
void process_fork(pid_t *pidp, void (*routine)(void*, void*), void *arg1, void *arg2)
{
    pid_t pid = fork();

    switch (pid)
    {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);

        case 0:
            srand((int) getpid());
            routine(arg1, arg2);
            return;

        default:
            *pidp = pid;
            return;
    }
}

/**
 * Generate a random number in an interval
 * TODO: low-order bits of modulus operation tend to influence the probability
 */
int rand_interval(int min, int max)
{
    return rand() % max + min;
}

/*
 * Sleep for t milliseconds
 */
int msleep(int t)
{
    struct timespec t1, t2;
    t1.tv_sec = 0;
    t2.tv_nsec = t*1000000L;
    return nanosleep(&t1, &t2);
}

/**
 * Allocates and initializes a POSIX named semaphore and shares it across forked
 * processes
 */
sem_t *semaphore_open(const char *name, unsigned int value)
{
    sem_t *sem = sem_open(name, O_CREAT, S_IRUSR | S_IWUSR, value);

    if (sem == SEM_FAILED)
    {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    return sem;
}

/**
 * Decrements the value of the semaphore and blocks the calling process if it
 * becomes negative
 */
void semaphore_wait(sem_t *sem)
{
    // TODO: consider sem_timedwait or sem_trywait
    if (sem_wait(sem) != 0)
    {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
}

/**
 * Increments the value of the semaphore and wakes up a blocked process waiting
 * on the semaphore, if any
 */
void semaphore_post(sem_t *sem)
{
    if (sem_post(sem) != 0)
    {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
}

/**
 * Closes a named semaphore, while still leaving it in shared memory
 */
void semaphore_close(sem_t *sem)
{
    if (sem_close(sem) != 0)
    {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
}

/**
 * Remove a named semaphore from shared memory, after all references are closed
 */
void semaphore_unlink(const char *name)
{
    if (sem_unlink(name) != 0)
    {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
}
