#include "threading.h"
// pthread_create(): Creates a new thread.
// pthread_join(): Waits for a thread to terminate.
// pthread_exit(): Terminates the calling thread.
// pthread_mutex_init(): Initializes a mutex.
// pthread_mutex_lock(): Locks a mutex.
// pthread_mutex_unlock(): Unlocks a mutex.
// pthread_cond_wait(): Waits for a condition variable.
// pthread_cond_signal(): Signals a condition variable.
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
//#define DEBUG_LOG(msg,...)
#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    pthread_mutex_t *mutex = thread_func_args->mutex;
    int wait_to_obtain_ms = thread_func_args->wait_to_obtain_ms;
    int wait_to_release_ms = thread_func_args->wait_to_release_ms;

    // wait & obtain mutex
    usleep(wait_to_obtain_ms * 1000);
    pthread_mutex_lock(mutex);
       
    // wait & release mutex
    usleep(wait_to_release_ms * 1000);
    pthread_mutex_unlock(mutex);
    
    thread_func_args->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    struct thread_data* thread_param = malloc(sizeof(struct thread_data));
    if (thread_param == NULL) {
        ERROR_LOG("Memory allocation failed");
        return false;
    }
    thread_param->mutex = mutex;
    thread_param->wait_to_obtain_ms = wait_to_obtain_ms;
    thread_param->wait_to_release_ms = wait_to_release_ms;
    thread_param->thread_complete_success = false;
    
    if (pthread_create(thread, NULL, threadfunc, thread_param) != 0) {
        ERROR_LOG("pthread_create failed");
	free(thread_param);
        return false;
    }

    return true;
}

