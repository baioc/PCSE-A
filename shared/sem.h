#ifndef _SEM_H_
#define _SEM_H_

/// Create a semaphore with a given initial value.
int screate(short int count);

/// Delete a semaphore and unblock any processes waiting on it.
int sdelete(int sem);

/// Reset a semaphore without invalidating it (unblocks processes).
int sreset(int sem, short int count);

/// V operation: Increment the semaphore count by one.
int signal(int sem);

/// V^n: Equivalent to COUNT signal()s, done atomically.
int signaln(int sem, short int count);

/// Test P operation on a semaphore, performing it only if it wouldn't block.
int try_wait(int sem);

/// P operation: Decrement the semaphore, blocking in case it goes negative.
int wait(int sem);

/**
 * Return the zero-extended 16-bit value of the given semaphore as a signed int.
 * If the provided sid is valid, returns 0xFFFFFFFF (aka -1).
 */
int scount(int sem);

#endif /* _SEM_H_ */
