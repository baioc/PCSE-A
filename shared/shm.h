#ifndef _SHM_H_
#define _SHM_H_

/**
 * Creates a shared memory page (4096 bytes long) with the given identifier.
 * If an object by that key already exists, returns NULL.
 */
void *shm_create(const char *key);

/**
 * Acquires an existing shared page, identified by the given key.
 * If that key doesn't exist, returns NULL.
 */
void *shm_acquire(const char *key);

/**
 * Releases a shared page, identified by the given key.
 * If that key doesn't exist, returns NULL.
 */
void shm_release(const char *key);

#endif /* _SHM_H_ */
