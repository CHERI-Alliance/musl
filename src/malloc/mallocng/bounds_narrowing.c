//TODO arm copyright ?
#define _BSD_SOURCE
/*
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include "glue.h"
*/
#include "meta.h"

#ifdef MORELLO

//TODO investigate thread safety

/*
 * This function attempt to find the wide capability related to the
 * user's capability. If used correctly, it should return the same
 * capability used in restrict_capability to get the user capability
 * This will check that the user didn't tamper with the index.
 * What happen on a corrupted index is yet undetermined, but in
 * no case will it return a valid capability on something else that
 * the correct wide capability
 */
void* get_wide_capability(void* user_capability)
{
	//TODO placeholder
	return user_capability;
}

/*
 * This function narrow the bounds of a wide capability and set up
 * the index in the userspace. It require the mapping to already
 * for the related group.
 */
void* restrict_capability(void* wide_capability, size_t user_size)
{
	//TODO placeholder
	return wide_capability;
}

/*
 * This function is to be called upon the creation of a new group before
 * it can be used. ie : one of its slot is given to the user. This adds an
 * entry to the map which allow one to get the wide pointer to the group
 * using an unique index saved in the group.
 * This is safe to call multiple time. If a mapping already exist, no new
 * one will be created
 */
void map_narrow_to_wide(void* wide_capability)
{
	// We need it in realloc as well, so we can't have it just defined in malloc.c

	// save the key in the group so that other slots use the same map. This also make sure we don't map twice the same group
}

/*
 * This clear the map created by map_narrow_to_wide. This is required to
 * avoid memory leak gven that group can be destroyed, which would lead
 * to wrong map entries.
 * This is safe to call multiple time. If the map is already cleared it
 * won't try to clear it again.
 */
void unmap_narrow_to_wide(void* wide_capability)
{
	// We need it in realloc as well, so we can't have it just defined in free.c

	// check that, if it is a multislot group, all slots are free before unmapping the group
}
#else
void* get_wide_capability(void* user_capability)
{
	return user_capability;
}

void* restrict_capability(void* wide_capability, size_t user_size)
{
	return wide_capability;
}

void map_narrow_to_wide(void* wide_capability) {}

void unmap_narrow_to_wide(void* wide_capability) {}
#endif