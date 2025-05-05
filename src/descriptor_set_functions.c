#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "descriptor_set_functions.h"
#include "info_struct.h"
#include "debug.h"

#ifdef DEBUG
int print_set_info(const struct rule rules[], struct set_info* set_info) {
	printf("R_set: %d[%d:%d]%d { ", set_info->r_lower_idx, set_info->r_size, set_info->r_alloc_size, set_info->r_higher_idx);
	if(!set_info->r_size) {
		printf("}\n");
		goto USET;
	}
	int i = set_info->r_lower_idx;
	printf("(%c, [%d[", set_info->R_set[i].rule, set_info->R_set[i].block_idx);
	for(int j = set_info->R_set[i].block_idx; j < set_info->R_set[i].block_end_idx; j++) {
		printf("%c", rules[set_info->R_set[i].rule - 65].blocks[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->R_set[i].block_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx, set_info->R_set[i].label_type);

	i = (i + 1) % set_info->r_alloc_size;
	while(i != set_info->r_higher_idx) {
		printf(", (%c, [%d[", set_info->R_set[i].rule, set_info->R_set[i].block_idx);
		for(int j = set_info->R_set[i].block_idx; j < set_info->R_set[i].block_end_idx; j++) {
			printf("%c", rules[set_info->R_set[i].rule - 65].blocks[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->R_set[i].block_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx, set_info->R_set[i].label_type);
		i = (i + 1) % set_info->r_alloc_size;
	}
	printf(" }\n");

USET:

	printf("U_set: %d[%d:%d]%d { ", set_info->u_lower_idx, set_info->u_size, set_info->u_alloc_size, set_info->u_higher_idx);
	if(!set_info->u_size) {
		printf("}\n");
		goto PSET;
	}
	i = set_info->u_lower_idx;
	printf("(%c, [%d[", set_info->U_set[i].rule, set_info->U_set[i].block_idx);
	for(int j = set_info->U_set[i].block_idx; j < set_info->U_set[i].block_end_idx; j++) {
		printf("%c", rules[set_info->U_set[i].rule - 65].blocks[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->U_set[i].block_end_idx, set_info->U_set[i].input_idx, set_info->U_set[i].gss_node_idx, set_info->U_set[i].label_type);

	i = (i + 1) % set_info->u_alloc_size;
	while(i != set_info->u_higher_idx) {
		printf(", (%c, [%d[", set_info->U_set[i].rule, set_info->U_set[i].block_idx);
		for(int j = set_info->U_set[i].block_idx; j < set_info->U_set[i].block_end_idx; j++) {
			printf("%c", rules[set_info->U_set[i].rule - 65].blocks[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->U_set[i].block_end_idx, set_info->U_set[i].input_idx, set_info->U_set[i].gss_node_idx, set_info->U_set[i].label_type);
		i = (i + 1) % set_info->u_alloc_size;
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
#endif

int in_set(
		const struct rule_info* rule_info,
		const descriptors U_set[],
		const uint16_t u_lower_idx,
		const uint16_t u_higher_idx,
		const uint16_t u_alloc_size,
		uint32_t input_idx,
		uint32_t gss_node_idx,
		uint8_t label_type
		) {

	assert(rule_info);
	assert(U_set);

	for(uint16_t i = u_higher_idx; i != u_lower_idx; i = (i + 1) % u_alloc_size) {
		if(
				U_set[i].rule == rule_info->rule &&
				U_set[i].block_idx == rule_info->start_idx &&
				U_set[i].input_idx == input_idx &&
				U_set[i].gss_node_idx == gss_node_idx
				) return i;
	}
	return -1;
}

int add_descriptor(
		const struct rule_info* rule_info,
		struct set_info* set_info,
		uint32_t input_idx,
		uint32_t gss_node_idx,
		uint8_t label_type
		){

	assert(rule_info);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(input_idx == set_info->lesser_input_idx || input_idx == set_info->lesser_input_idx + 1);

	if(in_set(
				rule_info,
				set_info->U_set,
				set_info->u_higher_idx,
				set_info->u_lower_idx,
				set_info->u_alloc_size,
				input_idx,
				gss_node_idx,
				label_type
				) != -1)
		return 0;

	// if we outgrow the current array resize it
	if(set_info->r_size >= set_info->r_alloc_size) {
		assert(set_info->r_lower_idx == set_info->r_higher_idx);
		realloc_descriptor_set(set_info, RSET);
	}

	// if we outgrow the current array resize it
	if(set_info->u_size >= set_info->u_alloc_size) {
		assert(set_info->u_lower_idx == set_info->u_higher_idx);
		realloc_descriptor_set(set_info, USET);
	}

	//add to R and U
	if(input_idx == set_info->lesser_input_idx) {
		if(set_info->r_lower_idx == 0)
			set_info->r_lower_idx = set_info->r_alloc_size - 1;
		else
			set_info->r_lower_idx -= 1;

		if(set_info->u_lower_idx == 0)
			set_info->u_lower_idx = set_info->u_alloc_size - 1;
		else
			set_info->u_lower_idx -= 1;

		set_info->R_set[set_info->r_lower_idx].rule = rule_info->rule;
		set_info->R_set[set_info->r_lower_idx].block_idx = rule_info->start_idx;
		set_info->R_set[set_info->r_lower_idx].block_end_idx = rule_info->end_idx;
		set_info->R_set[set_info->r_lower_idx].input_idx = input_idx;
		set_info->R_set[set_info->r_lower_idx].gss_node_idx = gss_node_idx;
		set_info->R_set[set_info->r_lower_idx].label_type = label_type;

		set_info->U_set[set_info->u_lower_idx].rule = rule_info->rule;
		set_info->U_set[set_info->u_lower_idx].block_idx = rule_info->start_idx;
		set_info->U_set[set_info->u_lower_idx].block_end_idx = rule_info->end_idx;
		set_info->U_set[set_info->u_lower_idx].input_idx = input_idx;
		set_info->U_set[set_info->u_lower_idx].gss_node_idx = gss_node_idx;
		set_info->U_set[set_info->u_lower_idx].label_type = label_type;
	} else {
		set_info->R_set[set_info->r_higher_idx].rule = rule_info->rule;
		set_info->R_set[set_info->r_higher_idx].block_idx = rule_info->start_idx;
		set_info->R_set[set_info->r_higher_idx].block_end_idx = rule_info->end_idx;
		set_info->R_set[set_info->r_higher_idx].input_idx = input_idx;
		set_info->R_set[set_info->r_higher_idx].gss_node_idx = gss_node_idx;
		set_info->R_set[set_info->r_higher_idx].label_type = label_type;

		set_info->U_set[set_info->u_higher_idx].rule = rule_info->rule;
		set_info->U_set[set_info->u_higher_idx].block_idx = rule_info->start_idx;
		set_info->U_set[set_info->u_higher_idx].block_end_idx = rule_info->end_idx;
		set_info->U_set[set_info->u_higher_idx].input_idx = input_idx;
		set_info->U_set[set_info->u_higher_idx].gss_node_idx = gss_node_idx;
		set_info->U_set[set_info->u_higher_idx].label_type = label_type;

		set_info->r_higher_idx = (set_info->r_higher_idx + 1) % set_info->r_alloc_size;
		set_info->u_higher_idx = (set_info->u_higher_idx + 1) % set_info->u_alloc_size;
	}
	set_info->r_size += 1;
	set_info->u_size += 1;
	return 0;
}

int clean_lesser_from_U(struct set_info* set_info) {
	uint16_t u_lower_idx = set_info->u_lower_idx;
	uint16_t u_size = set_info->u_size;
	uint16_t u_alloc_size = set_info->u_alloc_size;
	uint32_t lesser_input_idx = set_info->lesser_input_idx;
	descriptors* U_set = set_info->U_set;

	while(U_set[u_lower_idx].input_idx == lesser_input_idx && u_size > 0) {
		u_size -= 1;
		u_lower_idx = (u_lower_idx + 1) % u_alloc_size;
	}

	set_info->u_size = u_size;
	set_info->u_lower_idx = u_lower_idx;
	if(u_size != 0)
		set_info->lesser_input_idx += 1;
	return 0;
}

int is_in_p_set(const p_set_entry P_set[], const uint32_t p_size, uint32_t gss_node_idx, uint32_t input_idx) {
	assert(P_set);

	for(int i = 0; i < p_size; i++) {
		if(
				P_set[i].gss_node_idx == gss_node_idx &&
				P_set[i].input_idx == input_idx
			) return 1;
	}
	return 0;
}

int add_p_set_entry(struct set_info* set_info, uint32_t gss_node_idx, uint32_t input_idx) {
	if(is_in_p_set(set_info->P_set, set_info->p_size, gss_node_idx, input_idx)) return 1;

	// if we outgrow the current array resize it
	assert(set_info->P_set);
	if(set_info->p_size >= set_info->p_alloc_size) {
		set_info->p_alloc_size *= 2;
		set_info->P_set = (p_set_entry*) realloc(set_info->P_set, set_info->p_alloc_size * sizeof(p_set_entry));
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

descriptors* realloc_descriptor_set(struct set_info* set_info, set_type s) {
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(s < PSET);

	if(s == USET) {
		descriptors* buff = malloc(2 * set_info->u_alloc_size * sizeof(descriptors));
		assert(buff);

		uint16_t i = set_info->u_alloc_size >> 1;
		uint16_t j = set_info->u_lower_idx;
		//the first set needs to be unrolled to avoid triggering the while condition instandly
		buff[i++] = set_info->U_set[j];
		j = (j + 1) % set_info->u_alloc_size;
		while(j != set_info->u_higher_idx) {
			buff[i++] = set_info->U_set[j];
			j = (j + 1) % set_info->u_alloc_size;
		}
		free(set_info->U_set);
		set_info->U_set = buff;
		set_info->u_lower_idx = set_info->u_alloc_size >> 1;
		set_info->u_higher_idx = i;
		set_info->u_alloc_size *= 2;
	} else if (s == RSET) {
		descriptors* buff = malloc(2 * set_info->r_alloc_size * sizeof(descriptors));
		assert(buff);

		uint16_t i = set_info->r_alloc_size >> 1; //start the new buffer in the middle for cache locality
		uint16_t j = set_info->r_lower_idx;
		//the first set needs to be unrolled to avoid triggering the while condition instandly
		buff[i++] = set_info->R_set[j];
		j = (j + 1) % set_info->r_alloc_size;
		while(j != set_info->r_higher_idx) {
			buff[i++] = set_info->R_set[j];
			j = (j + 1) % set_info->r_alloc_size;
		}
		free(set_info->R_set);
		set_info->R_set = buff;
		set_info->r_lower_idx = set_info->r_alloc_size >> 1;
		set_info->r_higher_idx = i;
		set_info->r_alloc_size *= 2;
	}
	return 0;
}

p_set_entry* init_p_set_entry_set(uint32_t size) {
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
