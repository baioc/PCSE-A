#ifndef _SEM_H_
#define _SEM_H_

/**
 * Create a semaphore with a given initial value.
 * Returns its id on success or a negative value on failure.
 */
int screate(short int count);

/**
 * Delete a semaphore and unblock any processes waiting on it.
 * Returns a -1 in case the provided semaphore id doesn't exist, 0 otherwise.
 */
int sdelete(int sem);

/**
 * Reset a semaphore without deleting it (also unblocks processes).
 * Returns its id on success or a negative value on failure.
 */
int sreset(int sem, short int count);

/**
 * V operation: Increment the semaphore count by one (if it wouldn't overflow).
 * Returns -1 if sem id is invalid, -2 in case of overflow, 0 otherwise.
 */
int signal(int sem);

/// V^n: Equivalent to COUNT signal()s done atomically.
int signaln(int sem, short int count);

/**
 * Test P operation on a semaphore, performing it only if it wouldn't block.
 * Returns -1 if sem id is invalid, -2 in case of overflow, -3 if the operation
 * would have blocked and 0 otherwise.
 */
int try_wait(int sem);

/**
 * P operation: Decrement the semaphore, blocking in case it goes negative.
 * Returns -1 if the sem id is invalid, -2 if the operation would cause an
 * overflow, -3 if while waiting the semaphore was deleted, -4 if while waiting
 * the semaphore was reset and 0 otherwise.
 */
int wait(int sem);

/**
 * Return the zero-extended 16-bit count on the given semaphore.
 * If the provided sid is valid, returns -1.
 */
int scount(int sem);


/*
* Display the following information about all existing semaphores:
*    - their id
*    - waiting processes (pid and name)
*    - counter value
*/
void sinfo();

#endif /* _SEM_H_ */
