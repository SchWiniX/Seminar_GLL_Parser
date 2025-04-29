#include<assert.h>
#include<stdint.h>
#include <stdlib.h>
#include"descriptor_set_functions.h"

uint16_t r_size = 0;
uint16_t u_size = 0;
uint16_t p_size = 0;
uint16_t r_total_size = 128;
uint16_t u_total_size = 256;
uint16_t p_total_size = 256;

int in_set(
		descriptors U_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_idx,
		uint16_t grammar_slot_pos,
		uint16_t in_pos,
		uint16_t gss_node_idx
		) {

	assert(U_set);

	uint16_t i;
	for(i = 0; i < u_size; i++) {
		if(
				U_set[i].grammar_rule_idx == grammar_rule_idx &&
				U_set[i].grammar_slot_pos == grammar_slot_pos &&
				U_set[i].in_pos == in_pos &&
				U_set[i].gss_node_idx == gss_node_idx &&
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
		uint16_t grammar_slot_idx,
		uint16_t grammar_slot_pos,
		uint16_t in_pos,
		uint16_t gss_node_idx
		){

	assert(R_set);
	assert(U_set);

	if(in_set(U_set, grammar_rule_idx, grammar_slot_idx, grammar_slot_pos, in_pos, gss_node_idx) != -1)
		return 0;

	// if we outgrow the current array resize it
	if(r_size >= r_total_size) {
		r_total_size *= 2;
		R_set = (descriptors*) realloc(R_set, r_total_size * sizeof(descriptors));
		assert(R_set);
	}

	// if we outgrow the current array resize it
	if(u_size >= u_total_size) {
		u_total_size *= 2;
		U_set = (descriptors*) realloc(U_set, u_total_size * sizeof(descriptors));
		assert(U_set);
	}

	//add to R and U
	R_set[r_size].grammar_rule_idx = grammar_rule_idx;
	R_set[r_size].grammar_slot_idx = grammar_slot_idx;
	R_set[r_size].grammar_slot_pos = grammar_slot_pos;
	R_set[r_size].in_pos = in_pos;
	R_set[r_size].gss_node_idx = gss_node_idx;

	U_set[u_size].grammar_rule_idx = grammar_rule_idx;
	U_set[u_size].grammar_slot_idx = grammar_slot_idx;
	U_set[u_size].grammar_slot_pos = grammar_slot_pos;
	U_set[u_size].in_pos = in_pos;
	U_set[u_size].gss_node_idx = gss_node_idx;

	u_size += 1;
	return 0;
}

int add_p_set_entry(p_set_entry P_set[], uint16_t gss_node_idx, uint16_t in_pos) {
	// if we outgrow the current array resize it
	assert(P_set);
	if(p_size >= p_total_size) {
		p_total_size *= 2;
		P_set = (p_set_entry*) realloc(P_set, u_total_size * sizeof(p_set_entry));
		assert(P_set);
	}

	// add new p entry
	P_set[p_size].gss_node_idx = gss_node_idx;
	P_set[p_size].in_pos = in_pos;
	p_size += 1;

	return 0;
}

descriptors* init_descriptor_set(uint16_t size) {
	descriptors* set = (descriptors*) malloc(sizeof(descriptors) * size);
	assert(set);
	return set;
}

p_set_entry* init_p_set_entry_set(uint16_t size) {
	p_set_entry* set = (p_set_entry*) malloc(sizeof(p_set_entry) * size);
	assert(set);
	return set;
}

int free_desc_set(descriptors* set) {
	if(!set) return 1;
	free(set);
	return 0;
}

int free_p_set_entry_set(p_set_entry* set) {
	if(!set) return 1;
	free(set);
	return 0;
}
