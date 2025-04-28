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
	uint16_t grammar_rule_idx;
	uint16_t grammar_slot_idx;
	uint16_t grammar_slot_pos;
	uint16_t gss_node_idx;
	uint16_t in_pos;
} descriptors;

typedef struct p_set_entry {
	uint16_t gss_node_idx;
	uint16_t in_pos;
} p_set_entry;

int in_set(
		descriptors U_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_idx,
		uint16_t grammar_slot_pos,
		uint16_t in_pos,
		uint16_t gss_node_id
		);

int add_descriptor(
		descriptors R_set[],
		descriptors U_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_idx,
		uint16_t grammar_slot_pos,
		uint16_t in_pos,
		uint16_t gss_node_idx
		);

int add_p_set_entry(
		p_set_entry p_set[],
		uint16_t gss_node_idx,
		uint16_t in_pos
		);

descriptors* init_set(uint16_t size);

int free_desc_set(descriptors* set);

#endif
