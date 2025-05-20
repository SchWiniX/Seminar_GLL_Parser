#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "descriptor_set_functions.h"
#include "gss.h"
#include "info_struct.h"
#include "debug.h"

#ifdef DEBUG
int print_set_info(const struct rule rules[], struct set_info* set_info, struct input_info* input_info, struct gss_info* gss_info, const uint8_t rule_count) {
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
	printf("]%d], %d, (%c, %d), %d)", set_info->R_set[i].alternative_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx.rule, set_info->R_set[i].gss_node_idx.input_idx, set_info->R_set[i].label_type);

	i = (i + 1) % set_info->r_alloc_size;
	while(i != set_info->r_higher_idx) {
		printf(", (%c, [%d[", set_info->R_set[i].rule, set_info->R_set[i].alternative_start_idx);
		for(int j = set_info->R_set[i].alternative_start_idx; j < set_info->R_set[i].alternative_end_idx; j++) {
			printf("%c", rules[set_info->R_set[i].rule - 'A'].alternatives[j]);
		}
		printf("]%d], %d, (%c, %d), %d)", set_info->R_set[i].alternative_end_idx, set_info->R_set[i].input_idx, set_info->R_set[i].gss_node_idx.rule, set_info->R_set[i].gss_node_idx.input_idx, set_info->R_set[i].label_type);
		i = (i + 1) % set_info->r_alloc_size;
	}
	printf(" }\n");

USET:

	printf("U_set:\n");
	for(int j = 0; j < rule_count + 2; j++) {
		for(int k = 0; k < input_info->input_size + 1; k++) {
			if(!(gss_info->gss[j * (input_info->input_size + 1) + k].U_set)) continue;
			if(!(gss_info->gss[j * (input_info->input_size + 1) + k].u_size)) continue;

			unsigned char r = (unsigned char) 255;
			for(int n = 0; n < 28; n++) {
				if(rules[n].name != n + 'A') continue;
				if(rules[n].count_idx == j) r = n + 'A';
			}
			assert(r != 255);

			gss_node g = gss_info->gss[j * (input_info->input_size + 1) + k];
			printf("{ '%c', %d, [%d], <", r, k, g.u_size);
			i = g.u_lower_idx;
			printf("(%c, [%d[", g.U_set[i].rule, g.U_set[i].alternative_start_idx);
			for(int j = g.U_set[i].alternative_start_idx; j < g.U_set[i].alternative_end_idx; j++) {
				printf("%c", rules[g.U_set[i].rule - 'A'].alternatives[j]);
			}
			printf("]%d], %d, %d)", g.U_set[i].alternative_end_idx, g.U_set[i].input_idx, g.U_set[i].label_type);
			
			i = (i + 1) % g.u_alloc_size;
			while(i != g.u_higher_idx) {
				printf(", (%c, [%d[", g.U_set[i].rule, g.U_set[i].alternative_start_idx);
				for(int j = g.U_set[i].alternative_start_idx; j < g.U_set[i].alternative_end_idx; j++) {
					printf("%c", rules[g.U_set[i].rule - 'A'].alternatives[j]);
				}
				printf("]%d], %d, %d)", g.U_set[i].alternative_end_idx, g.U_set[i].input_idx, g.U_set[i].label_type);
				i = (i + 1) % g.u_alloc_size;
			}
			printf("> }\n");
		}
	}

// PSET

	printf("P_set:\n");
	for(int j = 0; j < rule_count + 2; j++) {
		for(int k = 0; k < input_info->input_size + 1; k++) {
			if(!gss_info->gss[j * (input_info->input_size + 1) + k].u_size) continue;

			unsigned char r = (unsigned char) 255;
			for(int n = 0; n < 28; n++) {
				if(rules[n].name != n + 'A') continue;
				if(rules[n].count_idx == j) r = n + 'A';
			}
			assert(r != 255);


			gss_node g = gss_info->gss[j * (input_info->input_size + 1) + k];
			printf("{ '%c', %d, [%d], <", r, k, g.p_count);

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
	assert(gss_node->U_set);
	assert(gss_node->u_size > 0);
	assert(gss_node->U_set[gss_node->u_lower_idx].input_idx < lesser_input_idx);

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
		if(gss_node->U_set[i].input_idx + 1 == lesser_input_idx) {
			gss_node->u_lower_idx = (i + 1) % gss_node->u_alloc_size;
			return ret;
		}
		if(
				gss_node->U_set[i].rule == rule_info->rule &&
				gss_node->U_set[i].alternative_start_idx == rule_info->alternative_start_idx &&
				gss_node->U_set[i].input_idx == input_idx
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

	gss_node* gss = gss_info->gss;
	uint32_t input_size = input_info->input_size;
	uint32_t input_idx = input_info->input_idx;
	uint64_t gss_idx = GET_GSS_IDX(rule_info->rules, gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_size);

	const uint32_t init_u_arrey_size = 32;
	if(!gss[gss_idx].U_set) {
		gss[gss_idx].U_set = init_u_descriptor_set(init_u_arrey_size);
		gss[gss_idx].u_size = 0;
		gss[gss_idx].u_lower_idx = init_u_arrey_size >> 1;
		gss[gss_idx].u_higher_idx = init_u_arrey_size >> 1;
		gss[gss_idx].u_alloc_size = init_u_arrey_size;
	}

	if(
			gss[gss_idx].u_size > 0 &&
			gss[gss_idx].U_set[gss[gss_idx].u_lower_idx].input_idx < set_info->lesser_input_idx &&
			in_set_and_clean(
				rule_info,
				gss + gss_idx,
				set_info->lesser_input_idx,
				input_idx,
				gss_info->gss_node_idx,
				label_type
			) != -1)
		return 0;
	else if(
			gss[gss_idx].u_size > 0 &&
			gss[gss_idx].U_set[gss[gss_idx].u_lower_idx].input_idx >= set_info->lesser_input_idx &&
			in_set(
				rule_info,
				gss[gss_idx].U_set,
				gss[gss_idx].u_lower_idx,
				gss[gss_idx].u_higher_idx,
				gss[gss_idx].u_alloc_size,
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
	if(gss[gss_idx].u_size >= gss[gss_idx].u_alloc_size) {
		assert(gss[gss_idx].u_lower_idx == gss[gss_idx].u_higher_idx);
		realloc_u_set(gss_info, rule_info, input_info, gss_info->gss_node_idx, set_info->lesser_input_idx);
	}

	descriptors* R_set = set_info->R_set;
	u_descriptors* U_set = gss[gss_idx].U_set;
	uint16_t r_index = 0;
	uint16_t u_index = 0;


	//add to R and U
	if(input_idx == set_info->lesser_input_idx) {
		if(set_info->r_lower_idx == 0)
			set_info->r_lower_idx = set_info->r_alloc_size - 1;
		else
			set_info->r_lower_idx -= 1;

		if(gss[gss_idx].u_lower_idx == 0)
			gss[gss_idx].u_lower_idx = gss[gss_idx].u_alloc_size - 1;
		else
			gss[gss_idx].u_lower_idx -= 1;
		r_index = set_info->r_lower_idx;
		u_index = gss[gss_idx].u_lower_idx;
	} else {
		r_index = set_info->r_higher_idx;
		u_index = gss[gss_idx].u_higher_idx;
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
		gss[gss_idx].u_higher_idx = (gss[gss_idx].u_higher_idx + 1) % gss[gss_idx].u_alloc_size;
	}
	set_info->r_size += 1;
	gss[gss_idx].u_size += 1;
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
		gss_info->gss + GET_GSS_IDX(rule_info->rules, gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_info->input_size);

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
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		const gss_node_idx new_node,
		const uint32_t new_edge,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(gss_info);
	assert(gss_info->gss);
	assert(set_info);

	struct gss_node* gss_node =
		gss_info->gss + GET_GSS_IDX(rule_info->rules, gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_info->input_size);

	assert(gss_node);
	assert(gss_node->p_entries);
	assert(gss_node->p_count >= 0 || gss_node->p_count <= 2);

	//for each entry in P that has src 'new_node'
	for(int i = 0; i < gss_node->p_count; i++) {
		if(gss_node->p_entries[i] < set_info->lesser_input_idx) { //invalid p_set
			gss_node->p_count = i;
			break;
		}
		gss_edge curr_edge = gss_info->gss[GET_GSS_IDX(rule_info->rules, new_node.rule, new_node.input_idx, input_info->input_size)].edge_arr[new_edge];

		struct rule_info r = {
			.rules = rule_info->rules,
			.rule = curr_edge.rule,
			.alternative_start_idx = curr_edge.alternative_start_idx,
			.alternative_end_idx = curr_edge.alternative_end_idx,
		};

		struct input_info in = {
			.input = NULL,
			.input_idx = gss_node->p_entries[i],
			.input_size = input_info->input_size,
		};

		assert(
				gss_node->p_entries[i] == set_info->lesser_input_idx ||
				gss_node->p_entries[i] + 1 == set_info->lesser_input_idx
				);

		add_descriptor(
				&r,
				&in,
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

u_descriptors* init_u_descriptor_set(const uint16_t size) {
	u_descriptors* set = (u_descriptors*) malloc(sizeof(u_descriptors) * size);
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

int realloc_u_set(
		struct gss_info* gss_info,
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		const gss_node_idx gss_node_idx,
		const uint32_t lesser_input_idx
		) {
	assert(gss_info);
	assert(gss_info->gss);

	gss_node* gss = gss_info->gss;
	uint32_t gss_idx = GET_GSS_IDX(rule_info->rules, gss_node_idx.rule, gss_node_idx.input_idx, input_info->input_size);

	u_descriptors* buff = malloc(2 * gss[gss_idx].u_alloc_size * sizeof(u_descriptors));
	assert(buff);

	uint16_t i = gss[gss_idx].u_alloc_size >> 1;
	uint16_t j = gss[gss_idx].u_lower_idx;
	//the first set needs to be unrolled to avoid triggering the while condition instandly
	buff[i++] = gss[gss_idx].U_set[j];
	j = (j + 1) % gss[gss_idx].u_alloc_size;
	while(j != gss[gss_idx].u_higher_idx) {
		buff[i++] = gss[gss_idx].U_set[j];
		j = (j + 1) % gss[gss_idx].u_alloc_size;
	}
	free(gss[gss_idx].U_set);
	gss[gss_idx].U_set = buff;
	gss[gss_idx].u_lower_idx = gss[gss_idx].u_alloc_size >> 1;
	gss[gss_idx].u_higher_idx = i;
	gss[gss_idx].u_alloc_size *= 2;

	return 0;
}

int free_desc_set(descriptors* set) {
	if(!set) return 1;
	free(set);
	return 0;
}

int free_u_desc_set(descriptors* set) {
	if(!set) return 1;
	free(set);
	return 0;
}
