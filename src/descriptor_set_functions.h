#ifndef DESCRIPTOR_SET_FUNCTIONS
#define DESCRIPTOR_SET_FUNCTIONS

#include<stdint.h>
#include"grammer_handler.h"

extern uint16_t r_size;
extern uint16_t u_size;
extern uint16_t p_size;
extern uint16_t r_alloc_size;
extern uint16_t u_alloc_size;
extern uint16_t p_alloc_size;

typedef struct descriptors {
	uint32_t input_idx;
	uint16_t block_idx;
	uint16_t block_end_idx;
	uint16_t gss_node_idx;
	uint8_t label_type;
	char rule;
} descriptors;

typedef struct p_set_entry {
	uint32_t input_idx;
	uint16_t gss_node_idx;
} p_set_entry;

struct set_info {
	descriptors* R_set;
	descriptors* U_set;
	p_set_entry* P_set;
};

int print_set_info(struct rule rules[], struct set_info* set_info);

int in_set(
		descriptors U_set[],
		uint16_t rule,
		uint16_t block_idx,
		uint16_t block_end_idx,
		uint32_t input_idx,
		uint16_t gss_node_idx,
		uint8_t label_type
		);

int add_descriptor(
		descriptors R_set[],
		descriptors U_set[],
		uint16_t rule,
		uint16_t block_idx,
		uint16_t block_end_idx,
		uint32_t input_idx,
		uint16_t gss_node_idx,
		uint8_t label_type
		);

int add_p_set_entry(
		p_set_entry p_set[],
		uint16_t gss_node_idx,
		uint32_t input_idx
		);

descriptors* init_descriptor_set(uint16_t size);
int free_desc_set(descriptors* set);

p_set_entry* init_p_set_entry_set(uint16_t size);
int free_p_set_entry_set(p_set_entry* set);
#endif
