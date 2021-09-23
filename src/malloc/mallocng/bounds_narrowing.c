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

size_t get_morello_alignment(size_t len)
{
	size_t res = ~__builtin_cheri_representable_alignment_mask(len) + 1;
	res = res < UNIT ? UNIT : res;
	return res;
}

void** get_cap_from_index(size_t global_index)
{
	if(!ctx.init_done) {
		//TODO error
		return NULL;
	}
	int current_max_index = (PGSZ << ctx.allocated_map_table_count) - PGSZ;

	if (global_index <= 0 || global_index >= current_max_index) {
		//TODO error
		return NULL;
	}

	// here we find in which table to look for our index
	unsigned int meta_index = 0;
	unsigned int tmp_index = global_index / PGSZ;
	while(tmp_index) {
		tmp_index >>= 1;
		meta_index++;
	}

	// index in this particular map table
	unsigned int local_index = global_index - ((PGSZ << meta_index) - PGSZ);

	return (ctx.capability_map_meta_table[meta_index] + local_index);
}

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
	unsigned int index = *((unsigned int*) ((unsigned char*)user_capability - MAP_KEY_OFFSET));

	unsigned char* tentative_group_capability = *get_cap_from_index(index);

	// Now we have to make sure this group capability match the one the user gave us
	// this means the user's capability is in this group.

	//TODO more test : test if it actually match a slot's (offsetted) start
	//TODO also test if the user's bounds or permissions have been further narrowed (ie : not the original returned by malloc)
	// https://github.com/capablevms/cheri_misidioms/blob/master/cheri_misidioms.ltx#L88

	unsigned int offset = ((unsigned char*)user_capability - MAP_KEY_OFFSET) - tentative_group_capability;

	// return a capability with the bounds allowing the full group, but with the address
	// being the same as the user's provided one so that it looks like we just expanded the bounds
	void* wide_capability = tentative_group_capability + offset;

	assert(__builtin_cheri_tag_get(wide_capability));
	return wide_capability;
}

/*
 * This function narrow the bounds of a wide capability and set up
 * the index in the userspace. It require the mapping to already
 * for the related group.
 */
void* restrict_capability(void* wide_capability, size_t user_size)
{
	//TODO placeholder
/*
	there is more than just the bound to strip
P 21-22 : https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-932.pdf
we strip the vmmap tag from returned pointers to prevent
the protections or mappings of the underlying memory from being changed by the process
*/
	struct meta *meta = get_meta(wide_capability);
	*((unsigned int*) wide_capability) = meta->mem->capability_map_index;

	//We expect enough memory to have been reserved so that the upper bound won't hit anything
	void* user_p = __builtin_cheri_bounds_set(wide_capability, user_size + MAP_KEY_OFFSET);

	return (unsigned char*)user_p + MAP_KEY_OFFSET;
}

/*
 * This function is to be called upon the creation of a new group before
 * it can be used. ie : one of its slot is given to the user. This adds an
 * entry to the map which allow one to get the wide pointer to the group
 * using an unique index saved in the group.
 * This is safe to call multiple time. If a mapping already exist, no new
 * one will be created
 */
size_t map_narrow_to_wide(void* wide_capability)
{
	struct meta* meta = get_meta(wide_capability);
	struct group* group = meta->mem;

	if (group->capability_map_index == GROUP_MAP_NOT_SET) {
		struct group** free_slot = get_cap_from_index(ctx.map_count);

		if (free_slot == 0) {
			// Initialize the next table holding the capability to the groups
			void* new_map_table = mmap(0,
					PGSZ << ctx.allocated_map_table_count,
					PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
				if (new_map_table==MAP_FAILED) {
					return GROUP_MAP_NOT_SET;
				}
			ctx.capability_map_meta_table[ctx.allocated_map_table_count] = new_map_table;
			ctx.allocated_map_table_count++;
			// Retry getting a slot
			free_slot = get_cap_from_index(ctx.map_count);
		}
		group->capability_map_index = ctx.map_count;
		*free_slot = group;
		ctx.map_count++;
	}
	assert(group == *get_cap_from_index(group->capability_map_index));

	return group->capability_map_index;
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
	// check, if it is a multislot group, is all slots are free before unmapping the group
	struct meta* meta = get_meta(wide_capability);
	if(meta->mem->capability_map_index == GROUP_MAP_NOT_SET) {
		return; //prevent double unmap of the same group
	}

	//if one bit of avail and free are both at 0, this mean the slot is currently in use
	//last_idx gives the number of bit to consider : not all of the 32 slots are neccessarly
	//active and relevant. This gives us a bool weither any other slot are currently in use
	uint32_t self = 1u<<get_slot_index(wide_capability);
	uint32_t mask = meta->freed_mask | meta->avail_mask;

	if (meta->sizeclass > 48 || mask+self == (2u<<meta->last_idx)-1) {
		// Then either it's a large group with a single slot, or no other slot than the
		// current on is in use : we can unmap

		if(meta->mem->capability_map_index != ctx.map_count-1) {
			unsigned int hole_index = meta->mem->capability_map_index;
			unsigned int replacement_index = ctx.map_count-1;
			void** map_table_hole_slot = get_cap_from_index(hole_index);
			struct group* moving_group = *get_cap_from_index(replacement_index);
			moving_group->capability_map_index = hole_index;
			*map_table_hole_slot = moving_group;
			for (int idx = 0; idx <= moving_group->meta->last_idx; idx++) {
				size_t stride = get_stride(moving_group->meta);
				unsigned char *start = moving_group->storage + stride*idx;
				unsigned int offset = 0;
				if (*(start-3) == 0b11100000) {
					offset = (*(uint16_t *)(start-2)) * UNIT;
				}
				*(start + offset) = hole_index;
			}
		}

		ctx.map_count--;
		assert(ctx.map_count > 0);
		meta->mem->capability_map_index = GROUP_MAP_NOT_SET;
	}
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

size_t map_narrow_to_wide(void* wide_capability) {}

void unmap_narrow_to_wide(void* wide_capability) {}
#endif