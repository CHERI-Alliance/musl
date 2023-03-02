#ifndef _CMPT_H
#define _CMPT_H
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

//NOTE: Experimental, possibly will be subject to significant change

typedef struct
{
	void* code;
	void* data;
	void* target;
} cap_pair_t; //TODO: If target kept in this struct, think of more accurate name

int create_cap_pair(cap_pair_t* dst, char* data, size_t data_size, void* target_func);
void call_to_cap_pair(cap_pair_t*);
#ifdef __cplusplus
}
#endif
#endif
