#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include "info_struct.h"
#include "debug.h"

#ifdef DEBUG
int print_set_info(const struct rule_arr rule_arr, struct set_info* set_info, struct input_info* input_info, struct gss_info* gss_info) {
	printf("R_set: %d[%d:%d]%d { ", set_info->r_lower_idx, set_info->r_size, set_info->r_alloc_size, set_info->r_higher_idx);
	int i = 0;
	if(!set_info->r_size) {
		printf("}\n");
		goto USET;
	}
	i = set_info->r_lower_idx;
	printf("(%s, [%d[", rule_arr.rules[set_info->R_set[i].rule].name, set_info->R_set[i].alternative_start_idx);
	for(uint16_t j = set_info->R_set[i].alternative_start_idx; j < set_info->R_set[i].alternative_end_idx; j++) {
		if(!is_non_terminal(rule_arr.rules[set_info->R_set[i].rule].alternatives + j))
			printf("%c", rule_arr.rules[set_info->R_set[i].rule].alternatives[j]);
		else {
			uint16_t rule_idx = token_to_idx(rule_arr.rules[set_info->R_set[i].rule].alternatives + j, &j);
			printf("'%s'", rule_arr.rules[rule_idx].name);
		}
	}
	printf("]%d], %d, (%s, %d), %d)", set_info->R_set[i].alternative_end_idx, set_info->R_set[i].input_idx, rule_arr.rules[set_info->R_set[i].gss_node_idx.rule].name, set_info->R_set[i].gss_node_idx.input_idx, set_info->R_set[i].label_type);

	i = (i + 1) % set_info->r_alloc_size;
	while(i != set_info->r_higher_idx) {
		printf(", (%s, [%d[", rule_arr.rules[set_info->R_set[i].rule].name, set_info->R_set[i].alternative_start_idx);
		for(uint16_t j = set_info->R_set[i].alternative_start_idx; j < set_info->R_set[i].alternative_end_idx; j++) {
			if(!is_non_terminal(rule_arr.rules[set_info->R_set[i].rule].alternatives + j))
				printf("%c", rule_arr.rules[set_info->R_set[i].rule].alternatives[j]);
			else {
				uint16_t rule_idx = token_to_idx(rule_arr.rules[set_info->R_set[i].rule].alternatives + j, &j);
				printf("'%s'", rule_arr.rules[rule_idx].name);
			}
		}
		printf("]%d], %d, (%s, %d), %d)", set_info->R_set[i].alternative_end_idx, set_info->R_set[i].input_idx, rule_arr.rules[set_info->R_set[i].gss_node_idx.rule].name, set_info->R_set[i].gss_node_idx.input_idx, set_info->R_set[i].label_type);
		i = (i + 1) % set_info->r_alloc_size;
	}
	printf(" }\n");

USET:

	printf("U_set:\n");
	for(int j = 0; j < rule_arr.rule_size + 2; j++) {
		for(int k = 0; k < input_info->input_size + 1; k++) {
			if(!gss_info->gss[GET_GSS_IDX(j, k, input_info->input_size)]) continue;

			gss_node* g = gss_info->gss[GET_GSS_IDX(j, k, input_info->input_size)];
			u_descriptors* U_set = GET_GSS_USET(g);
			printf("{ '%s', %d, %d[%d:%d]%d, <", rule_arr.rules[j].name, k, g->u_lower_idx, g->u_size, g->u_alloc_size, g->u_higher_idx);
			i = g->u_lower_idx;
			if(g->u_size > 0) {
				printf("(%s, [%d[", rule_arr.rules[U_set[i].rule].name, U_set[i].alternative_start_idx);
				for(uint16_t j = U_set[i].alternative_start_idx; j < U_set[i].alternative_end_idx; j++) {
					if(!is_non_terminal(rule_arr.rules[U_set[i].rule].alternatives + j))
						printf("%c", rule_arr.rules[U_set[i].rule].alternatives[j]);
					else {
						uint16_t rule_idx = token_to_idx(rule_arr.rules[U_set[i].rule].alternatives + j, &j);
						printf("'%s'", rule_arr.rules[rule_idx].name);
					}
				}
				printf("]%d], %d, %d)", U_set[i].alternative_end_idx, U_set[i].input_idx, U_set[i].label_type);
				
				i = (i + 1) % g->u_alloc_size;
				while(i != g->u_higher_idx) {
					printf(", (%s, [%d[", rule_arr.rules[U_set[i].rule].name, U_set[i].alternative_start_idx);
					for(uint16_t j = U_set[i].alternative_start_idx; j < U_set[i].alternative_end_idx; j++) {
						if(!is_non_terminal(rule_arr.rules[U_set[i].rule].alternatives + j))
							printf("%c", rule_arr.rules[U_set[i].rule].alternatives[j]);
						else {
							uint16_t rule_idx = token_to_idx(rule_arr.rules[U_set[i].rule].alternatives + j, &j);
							printf("'%s'", rule_arr.rules[rule_idx].name);
						}
					}
					printf("]%d], %d, %d)", U_set[i].alternative_end_idx, U_set[i].input_idx, U_set[i].label_type);
					i = (i + 1) % g->u_alloc_size;
				}
			}
			printf("> }\n");
		}
	}

// PSET

	printf("P_set:\n");
	for(int j = 0; j < rule_arr.rule_size + 2; j++) {
		for(int k = 0; k < input_info->input_size + 1; k++) {
			if(!gss_info->gss[j * (input_info->input_size + 1) + k]) continue;

			gss_node g = *(gss_info->gss[GET_GSS_IDX(j, k, input_info->input_size)]);
			printf("{ '%s', %d, [%d], <", rule_arr.rules[j].name, k, g.p_count);

			if(g.p_count > 0)
				printf("%d", g.p_entries[0]);
			if(g.p_count > 1)
				printf(", %d", g.p_entries[1]);

			printf("> }\n");
		}
	}

	return 0;
}
#endif

// assumes cleaning to be nessessary
int in_set_and_clean(
		const struct rule_info* rule_info,
		struct gss_node* gss_node,
		const uint32_t lesser_input_idx,
		uint32_t input_idx,
		gss_node_idx gss_node_idx,
		uint8_t label_type
		) {

	assert(rule_info);
	assert(gss_node);

	u_descriptors* U_set = GET_GSS_USET(gss_node);
	assert(U_set);
	assert(gss_node->u_size > 0);
	assert(U_set[gss_node->u_lower_idx].input_idx < lesser_input_idx);

	//u_set is all invalid hence clean it
	if(lesser_input_idx < input_idx + 1) { 
		gss_node->u_size = 0;
		gss_node->u_lower_idx = gss_node->u_alloc_size >> 1;
		gss_node->u_higher_idx = gss_node->u_alloc_size >> 1;
		return -1;
	}
	//else we assume that the higher halve is still valid
	uint32_t ret = -1;
	uint32_t i;
	if(gss_node->u_higher_idx == 0) i = gss_node->u_alloc_size - 1;
	else i = gss_node->u_higher_idx - 1;
	uint32_t j;
	if(gss_node->u_lower_idx == 0) j = gss_node->u_alloc_size - 1;
	else j = gss_node->u_lower_idx - 1;
	while(i != j) {
		if(U_set[i].input_idx + 1 == lesser_input_idx) {
			gss_node->u_lower_idx = (i + 1) % gss_node->u_alloc_size;
			return ret;
		}
		if(
				U_set[i].rule == rule_info->rule &&
				U_set[i].alternative_start_idx == rule_info->alternative_start_idx &&
				U_set[i].input_idx == input_idx
				) ret = i;
		if(i == 0) i = gss_node->u_alloc_size - 1;
		else i -= 1;
	}
	return ret;
}

int in_set(
		const struct rule_info* rule_info,
		const u_descriptors U_set[],
		const uint16_t u_lower_idx,
		const uint16_t u_higher_idx,
		const uint16_t u_alloc_size,
		const uint32_t lesser_input_idx,
		uint32_t input_idx,
		gss_node_idx gss_node_idx,
		uint8_t label_type
		) {

	assert(rule_info);
	assert(U_set);

	assert(lesser_input_idx == input_idx || lesser_input_idx + 1 == input_idx);
	if(lesser_input_idx == input_idx) {
		for(uint16_t i = u_lower_idx; i != u_higher_idx; i = (i + 1) % u_alloc_size) {
			if(U_set[i].input_idx > lesser_input_idx) return -1;
			if(
					U_set[i].rule == rule_info->rule &&
					U_set[i].alternative_start_idx == rule_info->alternative_start_idx &&
					U_set[i].input_idx == input_idx
					) return i;
		}
	} else {
		uint32_t i;
		if(u_higher_idx == 0) i = u_alloc_size - 1;
		else i = u_higher_idx - 1;
		uint32_t j;
		if(u_lower_idx == 0) j = u_alloc_size - 1;
		else j = u_lower_idx - 1;
		while(i != j) {
			if(U_set[i].input_idx == lesser_input_idx) return -1;
			if(
					U_set[i].rule == rule_info->rule &&
					U_set[i].alternative_start_idx == rule_info->alternative_start_idx &&
					U_set[i].input_idx == input_idx
					) return i;
			if(i == 0) i = u_alloc_size - 1;
			else i -= 1;
		}
	}
	return -1;
}

int add_descriptor(
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		struct set_info* set_info,
		struct gss_info* gss_info,
		uint8_t label_type
		) {

	assert(rule_info);
	assert(gss_info);
	assert(gss_info->gss);
	assert(set_info);
	assert(set_info->R_set);
	assert(input_info->input_idx == set_info->lesser_input_idx || input_info->input_idx == set_info->lesser_input_idx + 1);

	gss_node** gss = gss_info->gss;
	uint32_t input_size = input_info->input_size;
	uint32_t input_idx = input_info->input_idx;
	uint64_t gss_idx = GET_GSS_IDX(gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_size);

	assert(gss[gss_idx]);

	u_descriptors* U_set = GET_GSS_USET(gss[gss_idx]);
	if(
			gss[gss_idx]->u_size > 0 &&
			U_set[gss[gss_idx]->u_lower_idx].input_idx < set_info->lesser_input_idx &&
			in_set_and_clean(
				rule_info,
				gss[gss_idx],
				set_info->lesser_input_idx,
				input_idx,
				gss_info->gss_node_idx,
				label_type
			) != -1)
		return 0;
	else if(
			gss[gss_idx]->u_size > 0 &&
			U_set[gss[gss_idx]->u_lower_idx].input_idx >= set_info->lesser_input_idx &&
			in_set(
				rule_info,
				U_set,
				gss[gss_idx]->u_lower_idx,
				gss[gss_idx]->u_higher_idx,
				gss[gss_idx]->u_alloc_size,
				set_info->lesser_input_idx,
				input_idx,
				gss_info->gss_node_idx,
				label_type
			) != -1)
		return 0;

	// if we outgrow the current array resize it
	if(set_info->r_size >= set_info->r_alloc_size) {
		assert(set_info->r_lower_idx == set_info->r_higher_idx);
		realloc_r_set(set_info);
	}

	// if we outgrow the current array resize it
	if(gss[gss_idx]->u_size >= gss[gss_idx]->u_alloc_size) {
		assert(gss[gss_idx]->u_lower_idx == gss[gss_idx]->u_higher_idx);
		gss[gss_idx] = realloc_gss_node_u_set(gss[gss_idx]);
	}

	uint16_t u_alloc_size = gss[gss_idx]->u_alloc_size;

	U_set = GET_GSS_USET(gss[gss_idx]);
	descriptors* R_set = set_info->R_set;
	uint16_t r_index = 0;
	uint16_t u_index = 0;


	//add to R and U
	if(input_idx == set_info->lesser_input_idx) {
		if(set_info->r_lower_idx == 0)
			set_info->r_lower_idx = set_info->r_alloc_size - 1;
		else
			set_info->r_lower_idx -= 1;

		if(gss[gss_idx]->u_lower_idx == 0)
			gss[gss_idx]->u_lower_idx = u_alloc_size - 1;
		else
			gss[gss_idx]->u_lower_idx -= 1;

		r_index = set_info->r_lower_idx;
		u_index = gss[gss_idx]->u_lower_idx;
	} else {
		r_index = set_info->r_higher_idx;
		u_index = gss[gss_idx]->u_higher_idx;
	}


	R_set[r_index].rule = rule_info->rule;
	R_set[r_index].alternative_start_idx = rule_info->alternative_start_idx;
	R_set[r_index].alternative_end_idx = rule_info->alternative_end_idx;
	R_set[r_index].input_idx = input_idx;
	R_set[r_index].gss_node_idx = gss_info->gss_node_idx;
	R_set[r_index].label_type = label_type;

	U_set[u_index].rule = rule_info->rule;
	U_set[u_index].alternative_start_idx = rule_info->alternative_start_idx;
	U_set[u_index].alternative_end_idx = rule_info->alternative_end_idx;
	U_set[u_index].input_idx = input_idx;
	U_set[u_index].label_type = label_type;

if(input_idx != set_info->lesser_input_idx) {
		set_info->r_higher_idx = (set_info->r_higher_idx + 1) % set_info->r_alloc_size;
		gss[gss_idx]->u_higher_idx = (gss[gss_idx]->u_higher_idx + 1) % u_alloc_size;
	}
	set_info->r_size += 1;
	gss[gss_idx]->u_size += 1;
	return 0;
}

int is_in_p_set(struct gss_node* gss_node, uint32_t input_idx, uint32_t lesser_input_idx) {

	assert(gss_node);
	assert(gss_node->p_entries);
	assert(gss_node->p_count == 1 || gss_node->p_count == 2);

	uint32_t* p_entries = gss_node->p_entries;
	uint8_t p_count = gss_node->p_count;

	//check validity
	if(p_entries[0] + 1 == lesser_input_idx && p_count == 2) {
		gss_node->p_entries[0] = p_entries[1];
		gss_node->p_count = 1;
	} else if (p_entries[0] + 1 == lesser_input_idx && p_count == 1) {
		gss_node->p_count = 0;
		return -1;
	} else if(p_entries[0] + 1 < lesser_input_idx) {
		gss_node->p_count = 0;
		return -1;
	}

	//set was valid search it
	if(p_entries[0] == input_idx) return 0;
	else if (p_count == 2 && p_entries[1] == input_idx) return 1; 
	
	return -1;
}

int add_p_set_entry(struct set_info* set_info, struct gss_info* gss_info, const struct rule_info* rule_info, const struct input_info* input_info, uint32_t input_idx) {

	assert(set_info);
	assert(gss_info);
	assert(input_info);
	assert(gss_info->gss);

	struct gss_node* gss_node =
		gss_info->gss[GET_GSS_IDX(gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_info->input_size)];
	assert(gss_node);

	if(
			gss_node->p_count > 0 &&
			is_in_p_set(
				gss_node,
				input_idx,
				set_info->lesser_input_idx
				) != -1) return 1;

	assert(gss_node->p_count == 1 || gss_node->p_count == 0);

	uint8_t p_count = gss_node->p_count;

	// add new p entry
	gss_node->p_entries[p_count] = input_idx;
	gss_node->p_count += 1;

	assert(gss_node->p_count == 2 || gss_node->p_count == 1);

	return 0;
}

int add_descriptor_for_P_set(
		struct rule_info* rule_info,
		struct input_info* input_info,
		const gss_node_idx new_node,
		const uint32_t new_edge,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(gss_info);
	assert(gss_info->gss);
	assert(set_info);

	struct gss_node* gss_node =
		gss_info->gss[GET_GSS_IDX(gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_info->input_size)];

	assert(gss_node);
	assert(gss_node->p_entries);
	assert(gss_node->p_count >= 0 || gss_node->p_count <= 2);

	//for each entry in P that has src 'new_node'
	gss_edge curr_edge = *(GET_GSS_EDGE_ARR(
					gss_info->gss[GET_GSS_IDX(new_node.rule, new_node.input_idx, input_info->input_size)]
					) + new_edge);
	for(int i = 0; i < gss_node->p_count; i++) {
		if(gss_node->p_entries[i] < set_info->lesser_input_idx) { //invalid p_set
			gss_node->p_count = i;
			break;
		}

		rule_info->rule = curr_edge.rule;
		rule_info->alternative_start_idx = curr_edge.alternative_start_idx;
		rule_info->alternative_end_idx = curr_edge.alternative_end_idx;

		input_info->input_idx = gss_node->p_entries[i],

		assert(
				gss_node->p_entries[i] == set_info->lesser_input_idx ||
				gss_node->p_entries[i] + 1 == set_info->lesser_input_idx
				);

		add_descriptor(
				rule_info,
				input_info,
				set_info,
				gss_info,
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

int realloc_r_set(struct set_info* set_info) {
	assert(set_info);
	assert(set_info->R_set);

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
	return 0;
}

int free_desc_set(descriptors* set) {
	if(!set) return 1;
	free(set);
	return 0;
}
