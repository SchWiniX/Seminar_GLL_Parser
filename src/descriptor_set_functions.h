#ifndef DESCRIPTOR_SET_FUNCTIONS
#define DESCRIPTOR_SET_FUNCTIONS

#include<stdint.h>

extern uint16_t r_size;
extern uint16_t u_size;
extern uint16_t p_size;
extern uint16_t r_total_size;
extern uint16_t u_total_size;
extern uint16_t p_total_size;

typedef struct descriptors {
	uint16_t rule;
	uint16_t block_idx;
	uint16_t gss_node_idx;
	uint16_t input_idx;
} descriptors;

typedef struct p_set_entry {
	uint16_t gss_node_idx;
	uint16_t input_idx;
} p_set_entry;

int in_set(
		descriptors U_set[],
		uint16_t rule,
		uint16_t block_idx,
		uint16_t input_idx,
		uint16_t gss_node_idx
		);

int add_descriptor(
		descriptors R_set[],
		descriptors U_set[],
		uint16_t rule,
		uint16_t block_idx,
		uint16_t input_idx,
		uint16_t gss_node_idx
		);

int add_p_set_entry(
		p_set_entry p_set[],
		uint16_t gss_node_idx,
		uint16_t input_idx
		);

descriptors* init_set(uint16_t size);

int free_desc_set(descriptors* set);

#endif
