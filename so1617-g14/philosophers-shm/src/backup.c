/**
 * Fetch a clean fork
 */
 void dining_room_fetch_clean_fork(Simulation *s)
 {
     sem_t *mutex_clean_forks = semaphore_open("/mutex_clean_forks", 1);
     sem_t *mutex_req_cutlery = semaphore_open("/mutex_req_cutlery", 1);
     sem_t *mutex_dirty_forks = semaphore_open("/mutex_dirty_forks",1);

     sem_t *signal_wash_cutlery = semaphore_open("/signal_wash_cutlery", 0);
     sem_t *signal_waiter = semaphore_open("/signal_waiter", 0);

     // Entering critical zone

     semaphore_wait(mutex_clean_forks);
     int cf = s->diningRoom->cleanForks;


     semaphore_wait(mutex_dirty_forks);
     int df = s->diningRoom->dirtyForks;


     if (cf == 0 && df > 0)
     {
         printf("Start waiter request\n");

         // Set cutlery request
         semaphore_wait(mutex_req_cutlery);
         s->waiter->reqCutlery = W_ACTIVE;
         semaphore_post(mutex_req_cutlery);

         // Wake up waiter
         semaphore_post(signal_waiter);

         printf("Waiting for clean forks...\n");

         // Wait for cutlery to be washed
         semaphore_wait(signal_wash_cutlery);

         s->diningRoom->cleanForks--;
     }
     else if(cf > 0)
     {
       s->diningRoom->cleanForks--;
     }

     // Leaving critical zone
     semaphore_post(mutex_dirty_forks);
     semaphore_post(mutex_clean_forks);


 }
