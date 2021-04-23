#include "userspace_apps.h"

#include "stddef.h"
#include "debug.h"
#include "hash.h"


static hash_t g_uapps;


// NOTE: while (void *) wouldn't usually be needed, its done to cast away const

void uapps_init(void)
{
	hash_init_string(&g_uapps);
	for (const struct uapps *uapp = symbols_table; uapp->name != NULL; ++uapp) {
		assert(uapp->start != NULL);
		assert(uapp->end != NULL);
		const int err = hash_set(&g_uapps, (void *)uapp->name, (void *)uapp);
		assert(!err);
	}
}

struct uapps *uapp_get(const char *name)
{
	return hash_get(&g_uapps, (void *)name, NULL);
}
