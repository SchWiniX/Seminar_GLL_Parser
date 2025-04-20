#include<assert.h>
#include<stdint.h>
#include <stdlib.h>
#include"descriptor_set_functions.h"

uint16_t u_size = 0;
uint16_t r_total_size = 128;
uint16_t u_total_size = 256;

int in_set(
		descriptors U_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_pos,
		uint16_t in_pos,
		uint16_t gss_node_idx,
		uint16_t sppf_node_idx,
		uint8_t grammar_slot_idx
		) {

	uint16_t i;
	for(i = 0; i < u_size; i++) {
		if(
				U_set[i].grammar_rule_idx == grammar_rule_idx &&
				U_set[i].grammar_slot_pos == grammar_slot_pos &&
				U_set[i].in_pos == in_pos &&
				U_set[i].gss_node_idx == gss_node_idx &&
				U_set[i].sppf_node_idx == sppf_node_idx &&
				U_set[i].grammar_slot_idx == grammar_slot_idx
				) {
			break;
		}
	}
	if(i == u_size)	return -1;
	return i;
}

int add_descriptor(
		descriptors R_set[],
		descriptors U_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_pos,
		uint16_t id,
		uint16_t in_pos,
		uint16_t gss_node_idx,
		uint16_t sppf_node_idx,
		uint8_t grammar_slot_idx
		){

	if(id >= r_total_size) {
		r_total_size *= 2;
		R_set = (descriptors*) realloc(R_set, r_total_size);
		assert(R_set);
	}

	if(u_size >= u_total_size) {
		u_total_size *= 2;
		U_set = (descriptors*) realloc(U_set, u_total_size);
		assert(U_set);
	}

	if(in_set(U_set, grammar_rule_idx, grammar_slot_pos, in_pos, gss_node_idx, sppf_node_idx, grammar_slot_idx) != -1)
		return 0;

	R_set[id].grammar_rule_idx = grammar_rule_idx;
	R_set[id].grammar_slot_pos = grammar_slot_pos;
	R_set[id].in_pos = in_pos;
	R_set[id].gss_node_idx = gss_node_idx;
	R_set[id].sppf_node_idx = sppf_node_idx;
	R_set[id].grammar_slot_idx = grammar_slot_idx;

	U_set[u_size].grammar_rule_idx = grammar_rule_idx;
	U_set[u_size].grammar_slot_pos = grammar_slot_pos;
	U_set[u_size].in_pos = in_pos;
	U_set[u_size].gss_node_idx = gss_node_idx;
	U_set[u_size].sppf_node_idx = sppf_node_idx;
	U_set[u_size].grammar_slot_idx = grammar_slot_idx;

	u_size += 1;
	return 0;
}

descriptors* init_set(uint16_t size) {
	descriptors* set = (descriptors*) malloc(sizeof(descriptors) * size);
	if(!set) return NULL;
	return set;
}

int free_desc_set(descriptors* set) {
	if(!set) return 1;
	free(set);
	return 0;
}
