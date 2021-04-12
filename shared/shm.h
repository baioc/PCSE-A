#ifndef _SHM_H_
#define _SHM_H_

/*
 * Create a shared memory object and stores it into a hash table with argument
 * key as the index.
 * If an object by that key already exists, return NULL.
 * Created objects are 4096 bytes long.
 */
void *shm_create(const char *key);

/*
 * Returns the address of memory object referenced by argument key.
 * If that key doesn't exist in shm_map, returns NULL.
 */
void *shm_acquire(const char *key);

/*
 * Delete a memory object referenced by key in shm_map hash table.
 * If that key doesn't exist in shm_map, does nothing.
 */
void shm_release(const char *key);

#endif /* _SHM_H_ */
