#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "descriptor_set_functions.h"
#include "info_struct.h"


int print_set_info(const struct rule rules[], struct set_info* set_info) {
	printf("R_set: [%d:%d] { ", set_info->r_size, set_info->r_alloc_size);
	if(set_info->r_size == 0) {
		printf("}\n");
		goto USET;
	}
	printf("(%c, [%d[", set_info->R_set[0].rule, set_info->R_set[0].block_idx);
	for(int j = set_info->R_set[0].block_idx; j < set_info->R_set[0].block_end_idx; j++) {
		printf("%c", rules[set_info->R_set[0].rule - 65].blocks[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->R_set[0].block_end_idx, set_info->R_set[0].input_idx, set_info->R_set[0].gss_node_idx, set_info->R_set[0].label_type);

	for(int i = 1; i < set_info->r_size; i++) {
		printf(", (%c, [%d[", set_info->R_set[i].rule, set_info->R_set[i].block_idx);
		for(int j = set_info->R_set[i].block_idx; j < set_info->R_set[i].block_end_idx; j++) {
			printf("%c", rules[set_info->R_set[i].rule - 65].blocks[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->R_set[i].block_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx, set_info->R_set[i].label_type);
	}
	printf(" }\n");

USET:

	printf("U_set: [%d:%d] { ", set_info->u_size, set_info->u_alloc_size);
	if(set_info->u_size == 0) {
		printf("}\n");
		goto PSET;
	}
	printf("(%c, [%d[", set_info->U_set[0].rule, set_info->U_set[0].block_idx);
	for(int j = set_info->U_set[0].block_idx; j < set_info->U_set[0].block_end_idx; j++) {
		printf("%c", rules[set_info->U_set[0].rule - 65].blocks[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->U_set[0].block_end_idx, set_info->U_set[0].input_idx, set_info->U_set[0].gss_node_idx, set_info->U_set[0].label_type);

	for(int i = 1; i < set_info->u_size; i++) {
		printf(", (%c, [%d[", set_info->U_set[i].rule, set_info->U_set[i].block_idx);
		for(int j = set_info->U_set[i].block_idx; j < set_info->U_set[i].block_end_idx; j++) {
			printf("%c", rules[set_info->U_set[i].rule - 65].blocks[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->U_set[i].block_end_idx, set_info->U_set[i].input_idx, set_info->U_set[i].gss_node_idx, set_info->U_set[i].label_type);
	}
	printf(" }\n");

PSET:

	printf("P_set: [%d:%d] { ", set_info->p_size, set_info->p_alloc_size);
	if(set_info->p_size == 0) {
		printf("}\n");
		return 0;
	}
	printf("(%d, %d)", set_info->P_set[0].input_idx, set_info->P_set[0].gss_node_idx);
	for(int i = 1; i < set_info->p_size; i++) {
		printf(", (%d, %d)", set_info->P_set[i].input_idx, set_info->P_set[i].gss_node_idx);
	}
	printf(" }\n");

	return 0;
}

int in_set(
		const struct rule_info* rule_info,
		const descriptors U_set[],
		const uint16_t u_size,
		uint32_t input_idx,
		uint16_t gss_node_idx,
		uint8_t label_type
		) {

	assert(rule_info);
	assert(U_set);

	uint16_t i;
	for(i = 0; i < u_size; i++) {
		if(
				U_set[i].rule == rule_info->rule &&
				U_set[i].block_idx == rule_info->start_idx &&
				U_set[i].input_idx == input_idx &&
				U_set[i].gss_node_idx == gss_node_idx
				) {
			break;
		}
	}
	if(i == u_size)	return -1;
	return i;
}

int add_descriptor(
		const struct rule_info* rule_info,
		struct set_info* set_info,
		uint32_t input_idx,
		uint16_t gss_node_idx,
		uint8_t label_type
		){

	assert(rule_info);
	assert(set_info);
	assert(set_info->U_set);
	assert(set_info->R_set);

	if(in_set(rule_info, set_info->U_set, set_info->u_size, input_idx, gss_node_idx, label_type) != -1)
		return 0;

	// if we outgrow the current array resize it
	if(set_info->r_size >= set_info->r_alloc_size) {
		set_info->r_alloc_size *= 2;
		set_info->R_set = (descriptors*) realloc(set_info->R_set, set_info->r_alloc_size * sizeof(descriptors));
		assert(set_info->R_set);
	}

	// if we outgrow the current array resize it
	if(set_info->u_size >= set_info->u_alloc_size) {
		set_info->u_alloc_size *= 2;
		set_info->U_set = (descriptors*) realloc(set_info->U_set, set_info->u_alloc_size * sizeof(descriptors));
		assert(set_info->U_set);
	}

	//add to R and U
	set_info->R_set[set_info->r_size].rule = rule_info->rule;
	set_info->R_set[set_info->r_size].block_idx = rule_info->start_idx;
	set_info->R_set[set_info->r_size].block_end_idx = rule_info->end_idx;
	set_info->R_set[set_info->r_size].input_idx = input_idx;
	set_info->R_set[set_info->r_size].gss_node_idx = gss_node_idx;
	set_info->R_set[set_info->r_size].label_type = label_type;

	set_info->U_set[set_info->u_size].rule = rule_info->rule;
	set_info->U_set[set_info->u_size].block_idx = rule_info->start_idx;
	set_info->U_set[set_info->u_size].block_end_idx = rule_info->end_idx;
	set_info->U_set[set_info->u_size].input_idx = input_idx;
	set_info->U_set[set_info->u_size].gss_node_idx = gss_node_idx;
	set_info->U_set[set_info->u_size].label_type = label_type;

	set_info->r_size += 1;
	set_info->u_size += 1;
	return 0;
}

int is_in_p_set(const p_set_entry P_set[], const uint16_t p_size, uint16_t gss_node_idx, uint32_t input_idx) {
	assert(P_set);

	for(int i = 0; i < p_size; i++) {
		if(
				P_set[i].gss_node_idx == gss_node_idx &&
				P_set[i].input_idx == input_idx
			) return 1;
	}
	return 0;
}

int add_p_set_entry(struct set_info* set_info, uint16_t gss_node_idx, uint32_t input_idx) {
	if(is_in_p_set(set_info->P_set, set_info->p_size, gss_node_idx, input_idx)) return 1;

	// if we outgrow the current array resize it
	assert(set_info->P_set);
	if(set_info->p_size >= set_info->p_alloc_size) {
		set_info->p_alloc_size *= 2;
		set_info->P_set = (p_set_entry*) realloc(set_info->P_set, set_info->u_alloc_size * sizeof(p_set_entry));
		assert(set_info->P_set);
	}

	// add new p entry
	set_info->P_set[set_info->p_size].gss_node_idx = gss_node_idx;
	set_info->P_set[set_info->p_size].input_idx = input_idx;
	set_info->p_size += 1;

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
