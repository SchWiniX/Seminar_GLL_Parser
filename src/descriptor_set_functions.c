#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include"descriptor_set_functions.h"
#include"grammer_handler.h"

uint16_t r_size = 0;
uint16_t u_size = 0;
uint16_t p_size = 0;
uint16_t r_alloc_size = 128;
uint16_t u_alloc_size = 256;
uint16_t p_alloc_size = 256;

int print_set_info(struct rule rules[], struct set_info* set_info) {
	printf("R_set: [%d:%d] { ", r_size, r_alloc_size);
	if(r_size == 0) {
		printf("}\n");
		goto USET;
	}
	printf("(%c, [%d[", set_info->R_set[0].rule, set_info->R_set[0].block_idx);
	for(int j = set_info->R_set[0].block_idx; j < set_info->R_set[0].block_end_idx; j++) {
		printf("%c", rules[set_info->R_set[0].rule - 65].blocks[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->R_set[0].block_end_idx, set_info->R_set[0].input_idx, set_info->R_set[0].gss_node_idx, set_info->R_set[0].label_type);

	for(int i = 1; i < r_size; i++) {
		printf(", (%c, [%d[", set_info->R_set[i].rule, set_info->R_set[i].block_idx);
		for(int j = set_info->R_set[i].block_idx; j < set_info->R_set[i].block_end_idx; j++) {
			printf("%c", rules[set_info->R_set[i].rule - 65].blocks[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->R_set[i].block_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx, set_info->R_set[i].label_type);
	}
	printf(" }\n");

USET:

	printf("U_set: [%d:%d] { ", u_size, u_alloc_size);
	if(u_size == 0) {
		printf("}\n");
		goto PSET;
	}
	printf("(%c, [%d[", set_info->U_set[0].rule, set_info->U_set[0].block_idx);
	for(int j = set_info->U_set[0].block_idx; j < set_info->U_set[0].block_end_idx; j++) {
		printf("%c", rules[set_info->U_set[0].rule - 65].blocks[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->U_set[0].block_end_idx, set_info->U_set[0].input_idx, set_info->U_set[0].gss_node_idx, set_info->U_set[0].label_type);

	for(int i = 1; i < u_size; i++) {
		printf(", (%c, [%d[", set_info->U_set[i].rule, set_info->U_set[i].block_idx);
		for(int j = set_info->U_set[i].block_idx; j < set_info->U_set[i].block_end_idx; j++) {
			printf("%c", rules[set_info->U_set[i].rule - 65].blocks[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->U_set[i].block_end_idx, set_info->U_set[i].input_idx, set_info->U_set[i].gss_node_idx, set_info->U_set[i].label_type);
	}
	printf(" }\n");

PSET:

	printf("P_set: [%d:%d] { ", p_size, p_alloc_size);
	if(p_size == 0) {
		printf("}\n");
		return 0;
	}
	printf("(%d, %d)", set_info->P_set[0].input_idx, set_info->P_set[0].gss_node_idx);
	for(int i = 1; i < p_size; i++) {
		printf(", (%d, %d)", set_info->P_set[i].input_idx, set_info->P_set[i].gss_node_idx);
	}
	printf(" }\n");

	return 0;
}

int in_set(
		descriptors U_set[],
		uint16_t rule,
		uint16_t block_idx,
		uint16_t block_end_idx,
		uint32_t input_idx,
		uint16_t gss_node_idx,
		uint8_t label_type
		) {

	assert(U_set);

	uint16_t i;
	for(i = 0; i < u_size; i++) {
		if(
				U_set[i].rule == rule &&
				U_set[i].block_idx == block_idx &&
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
		descriptors R_set[],
		descriptors U_set[],
		uint16_t rule,
		uint16_t block_idx,
		uint16_t block_end_idx,
		uint32_t input_idx,
		uint16_t gss_node_idx,
		uint8_t label_type
		){

	assert(R_set);
	assert(U_set);

	if(in_set(U_set, rule, block_idx, block_end_idx, input_idx, gss_node_idx, label_type) != -1)
		return 0;

	// if we outgrow the current array resize it
	if(r_size >= r_alloc_size) {
		r_alloc_size *= 2;
		R_set = (descriptors*) realloc(R_set, r_alloc_size * sizeof(descriptors));
		assert(R_set);
	}

	// if we outgrow the current array resize it
	if(u_size >= u_alloc_size) {
		u_alloc_size *= 2;
		U_set = (descriptors*) realloc(U_set, u_alloc_size * sizeof(descriptors));
		assert(U_set);
	}

	//add to R and U
	R_set[r_size].rule = rule;
	R_set[r_size].block_idx = block_idx;
	R_set[r_size].block_end_idx = block_end_idx;
	R_set[r_size].input_idx = input_idx;
	R_set[r_size].gss_node_idx = gss_node_idx;
	R_set[r_size].label_type = label_type;

	U_set[u_size].rule = rule;
	U_set[u_size].block_idx = block_idx;
	U_set[u_size].block_end_idx = block_end_idx;
	U_set[u_size].input_idx = input_idx;
	U_set[u_size].gss_node_idx = gss_node_idx;
	U_set[u_size].label_type = label_type;

	r_size += 1;
	u_size += 1;
	return 0;
}

int is_in_p_set(p_set_entry P_set[], uint16_t gss_node_idx, uint32_t input_idx) {
	assert(P_set);

	for(int i = 0; i < p_size; i++) {
		if(
				P_set[i].gss_node_idx == gss_node_idx &&
				P_set[i].input_idx == input_idx
			) return 1;
	}
	return 0;
}

int add_p_set_entry(p_set_entry P_set[], uint16_t gss_node_idx, uint32_t input_idx) {
	if(is_in_p_set(P_set, gss_node_idx, input_idx)) return 1;

	// if we outgrow the current array resize it
	assert(P_set);
	if(p_size >= p_alloc_size) {
		p_alloc_size *= 2;
		P_set = (p_set_entry*) realloc(P_set, u_alloc_size * sizeof(p_set_entry));
		assert(P_set);
	}

	// add new p entry
	P_set[p_size].gss_node_idx = gss_node_idx;
	P_set[p_size].input_idx = input_idx;
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
