#pragma once

inline static size_t cap_length(size_t req_length, size_t address)
{
	void *p = __builtin_cheri_cap_from_pointer(NULL, address);
	p = __builtin_cheri_bounds_set(p, req_length);
	return __builtin_cheri_length_get(p);
}

#define WOULD_BE_LENGTH(len, cap) cap_length(len, __builtin_cheri_address_get(cap))
