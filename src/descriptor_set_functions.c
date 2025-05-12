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
	int i = 0;
	if(!set_info->r_size) {
		printf("}\n");
		goto USET;
	}
	i = set_info->r_lower_idx;
	printf("(%c, [%d[", set_info->R_set[i].rule, set_info->R_set[i].alternative_start_idx);
	for(int j = set_info->R_set[i].alternative_start_idx; j < set_info->R_set[i].alternative_end_idx; j++) {
		printf("%c", rules[set_info->R_set[i].rule - 'A'].alternatives[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->R_set[i].alternative_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx, set_info->R_set[i].label_type);

	i = (i + 1) % set_info->r_alloc_size;
	while(i != set_info->r_higher_idx) {
		printf(", (%c, [%d[", set_info->R_set[i].rule, set_info->R_set[i].alternative_start_idx);
		for(int j = set_info->R_set[i].alternative_start_idx; j < set_info->R_set[i].alternative_end_idx; j++) {
			printf("%c", rules[set_info->R_set[i].rule - 'A'].alternatives[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->R_set[i].alternative_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx, set_info->R_set[i].label_type);
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
	printf("(%c, [%d[", set_info->U_set[i].rule, set_info->U_set[i].alternative_start_idx);
	for(int j = set_info->U_set[i].alternative_start_idx; j < set_info->U_set[i].alternative_end_idx; j++) {
		printf("%c", rules[set_info->U_set[i].rule - 'A'].alternatives[j]);
	}
	printf("]%d], %d, %d, %d)", set_info->U_set[i].alternative_end_idx, set_info->U_set[i].input_idx, set_info->U_set[i].gss_node_idx, set_info->U_set[i].label_type);

	i = (i + 1) % set_info->u_alloc_size;
	while(i != set_info->u_higher_idx) {
		printf(", (%c, [%d[", set_info->U_set[i].rule, set_info->U_set[i].alternative_start_idx);
		for(int j = set_info->U_set[i].alternative_start_idx; j < set_info->U_set[i].alternative_end_idx; j++) {
			printf("%c", rules[set_info->U_set[i].rule - 'A'].alternatives[j]);
		}
		printf("]%d], %d, %d, %d)", set_info->U_set[i].alternative_end_idx, set_info->U_set[i].input_idx, set_info->U_set[i].gss_node_idx, set_info->U_set[i].label_type);
		i = (i + 1) % set_info->u_alloc_size;
	}
	printf(" }\n");

PSET:

	printf("P_set: [%d:%d] { ", set_info->p_size, set_info->p_alloc_size);

	if(!set_info->p_size) {
		printf("}\n");
		return 0;
	}
	i = set_info->p_lower_idx;
	printf("(%d, %d)", set_info->P_set[i].input_idx, set_info->P_set[i].gss_node_idx);
	i = (i + 1) % set_info->p_alloc_size;
	while(i != set_info->p_higher_idx) {
		printf(", (%d, %d)", set_info->P_set[i].input_idx, set_info->P_set[i].gss_node_idx);
		i = (i + 1) % set_info->p_alloc_size;
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

	for(uint16_t i = u_lower_idx; i != u_higher_idx; i = (i + 1) % u_alloc_size) {
		if(
				U_set[i].rule == rule_info->rule &&
				U_set[i].alternative_start_idx == rule_info->alternative_start_idx &&
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
				set_info->u_lower_idx,
				set_info->u_higher_idx,
				set_info->u_alloc_size,
				input_idx,
				gss_node_idx,
				label_type
				) != -1)
		return 0;

	// if we outgrow the current array resize it
	if(set_info->r_size >= set_info->r_alloc_size) {
		assert(set_info->r_lower_idx == set_info->r_higher_idx);
		realloc_set(set_info, RSET);
	}

	// if we outgrow the current array resize it
	if(set_info->u_size >= set_info->u_alloc_size) {
		assert(set_info->u_lower_idx == set_info->u_higher_idx);
		realloc_set(set_info, USET);
	}

	descriptors* R_set = set_info->R_set;
	descriptors* U_set = set_info->U_set;
	uint16_t r_index = 0;
	uint16_t u_index = 0;


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
		r_index = set_info->r_lower_idx;
		u_index = set_info->u_lower_idx;
	} else {
		r_index = set_info->r_higher_idx;
		u_index = set_info->u_higher_idx;
	}


	R_set[r_index].rule = rule_info->rule;
	R_set[r_index].alternative_start_idx = rule_info->alternative_start_idx;
	R_set[r_index].alternative_end_idx = rule_info->alternative_end_idx;
	R_set[r_index].input_idx = input_idx;
	R_set[r_index].gss_node_idx = gss_node_idx;
	R_set[r_index].label_type = label_type;

	U_set[u_index].rule = rule_info->rule;
	U_set[u_index].alternative_start_idx = rule_info->alternative_start_idx;
	U_set[u_index].alternative_end_idx = rule_info->alternative_end_idx;
	U_set[u_index].input_idx = input_idx;
	U_set[u_index].gss_node_idx = gss_node_idx;
	U_set[u_index].label_type = label_type;

	if(input_idx != set_info->lesser_input_idx) {
		set_info->r_higher_idx = (set_info->r_higher_idx + 1) % set_info->r_alloc_size;
		set_info->u_higher_idx = (set_info->u_higher_idx + 1) % set_info->u_alloc_size;
	}
	set_info->r_size += 1;
	set_info->u_size += 1;
	return 0;
}

int clean_lesser_from_U(struct set_info* set_info) {

	assert(set_info);
	assert(set_info->U_set);

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

int clean_lesser_from_P(struct set_info* set_info) {

	assert(set_info);
	assert(set_info->P_set);

	uint16_t p_lower_idx = set_info->p_lower_idx;
	uint16_t p_size = set_info->p_size;
	uint16_t p_alloc_size = set_info->p_alloc_size;
	uint32_t lesser_input_idx = set_info->lesser_input_idx;
	p_set_entry* P_set = set_info->P_set;

	while(P_set[p_lower_idx].input_idx == lesser_input_idx && p_size > 0) {
		p_size -= 1;
		p_lower_idx = (p_lower_idx + 1) % p_alloc_size;
	}

	set_info->p_size = p_size;
	set_info->p_lower_idx = p_lower_idx;
	return 0;
}

int is_in_p_set(
		const p_set_entry P_set[],
		const uint32_t p_lower_idx,
		const uint32_t p_higher_idx,
		const uint32_t p_size,
		const uint32_t p_alloc_size,
		uint32_t gss_node_idx,
		uint32_t input_idx
		) {

	assert(P_set);
	if(p_size == 0) return 0;

	for(int i = p_lower_idx; i != p_higher_idx; i = (i + 1) % p_alloc_size) {
		if(
				P_set[i].gss_node_idx == gss_node_idx &&
				P_set[i].input_idx == input_idx
			) return 1;
	}
	return 0;
}

int add_p_set_entry(struct set_info* set_info, uint32_t gss_node_idx, uint32_t input_idx) {

	assert(set_info);
	assert(set_info->P_set);

	if(is_in_p_set(
				set_info->P_set,
				set_info->p_lower_idx,
				set_info->p_higher_idx,
				set_info->p_size,
				set_info->p_alloc_size,
				gss_node_idx,
				input_idx
				)) return 1;

	// if we outgrow the current array resize it
	if(set_info->p_size >= set_info->p_alloc_size) {
		realloc_set(set_info, PSET);
	}

	// add new p entry
	set_info->P_set[set_info->p_higher_idx].gss_node_idx = gss_node_idx;
	set_info->P_set[set_info->p_higher_idx].input_idx = input_idx;
	set_info->p_higher_idx = (set_info->p_higher_idx + 1) % set_info->p_alloc_size;
	set_info->p_size += 1;

	return 0;
}

int add_descriptor_for_P_set(const struct gss_info* gss_info, struct set_info* set_info, const uint32_t new_node, const uint32_t new_edge) {

	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(set_info);
	assert(set_info->P_set);

	uint32_t i;
	for(i = set_info->p_lower_idx; i < set_info->p_higher_idx; i = (i + 1) % set_info->p_alloc_size) {
		if(new_node != set_info->P_set[i].gss_node_idx) continue;
		gss_node curr_node = gss_info->gss_nodes[new_node];
		gss_edge curr_edge = gss_info->gss_edges[new_edge];
		struct rule_info r = {
			.rules = NULL,
			.rule = curr_node.rule,
			.alternative_start_idx = curr_edge.alternative_start_idx,
			.alternative_end_idx = curr_edge.alternative_end_idx,
		};
		assert(
				set_info->P_set[i].input_idx == set_info->lesser_input_idx ||
				set_info->P_set[i].input_idx + 1 == set_info->lesser_input_idx);

		add_descriptor(
				&r,
				set_info,
				set_info->P_set[i].input_idx,
				gss_info->gss_node_idx,
				curr_edge.label_type
				);
	}
	return 0;
}

descriptors* init_descriptor_set(const uint16_t size) {
	descriptors* set = (descriptors*) malloc(sizeof(descriptors) * size);
	assert(set);
	return set;
}

int realloc_set(struct set_info* set_info, const set_type s) {
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(s <= PSET);

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
	} else {
		p_set_entry* buff = malloc(2 * set_info->p_alloc_size * sizeof(p_set_entry));
		assert(buff);

		uint16_t i = set_info->p_alloc_size >> 1;
		uint16_t j = set_info->p_lower_idx;
		//the first set needs to be unrolled to avoid triggering the while condition instandly
		buff[i++] = set_info->P_set[j];
		j = (j + 1) % set_info->p_alloc_size;
		while(j != set_info->p_higher_idx) {
			buff[i++] = set_info->P_set[j];
			j = (j + 1) % set_info->p_alloc_size;
		}
		free(set_info->P_set);
		set_info->P_set = buff;
		set_info->p_lower_idx = set_info->p_alloc_size >> 1;
		set_info->p_higher_idx = i;
		set_info->p_alloc_size *= 2;
	}

	return 0;
}

p_set_entry* init_p_set_entry_set(const uint32_t size) {
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
