
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gss.h"
#include "grammer_handler.h"
#include "descriptor_set_functions.h"
#include "info_struct.h"
#include "debug.h"

const uint32_t init_gss_edge_arrey_size = 8;
const uint32_t init_u_arrey_size = 8;

#ifdef DEBUG
int print_gss_info(struct rule_arr rule_arr, struct gss_info* gss_info, struct input_info* input_info) {
	printf("gss (%s, %d):\n", rule_arr.rules[gss_info->gss_node_idx.rule].name, gss_info->gss_node_idx.input_idx);
	for(int j = 0; j < rule_arr.rule_size + 2; j++) {
		for(int i = 0; i < input_info->input_size + 1; i++) {
			if(!gss_info->gss[GET_GSS_IDX(j, i, input_info->input_size)]) continue;

			gss_node* g = gss_info->gss[GET_GSS_IDX(j, i, input_info->input_size)];
			gss_edge* edge_arr = GET_GSS_EDGE_ARR(g);

			printf("{ '%s', %d, <", rule_arr.rules[j].name, i);
			if(g->edge_size == 0) {
				printf("> }\n");
				continue;
			}
			for(int n = 0; n < g->edge_size; n++) {
				printf("(-> (%s, %d), %s, [%d[", rule_arr.rules[edge_arr[n].target_node.rule].name, edge_arr[n].target_node.input_idx, rule_arr.rules[edge_arr[n].rule].name, edge_arr[n].alternative_start_idx);
				for(uint16_t m = edge_arr[n].alternative_start_idx; m < edge_arr[n].alternative_end_idx; m++) {
					if(!is_non_terminal(rule_arr.rules[edge_arr[n].rule].alternatives + m))
						printf("%c", rule_arr.rules[edge_arr[n].rule].alternatives[m]);
					else {
						uint16_t rule_idx = token_to_idx(rule_arr.rules[edge_arr[n].rule].alternatives + m, &m);
						printf("'%s'", rule_arr.rules[rule_idx].name);
					}
				}
				printf("]%d], %d)", edge_arr[n].alternative_end_idx, edge_arr[n].label_type);
			}
			printf("> }\n");
		}
	}
	return 0;
}
#endif
 
gss_node_idx create(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info,
		uint8_t label_type
		) {

	assert(rule_info);
	assert(rule_info->rule_arr.rules);
	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(set_info);
	assert(set_info->R_set);

	gss_node** gss = gss_info->gss;
	uint32_t input_size = input_info->input_size;
	uint32_t input_idx = input_info->input_idx;
	uint16_t rule = rule_info->rule;
	uint64_t gss_idx = GET_GSS_IDX(rule, input_idx, input_size);
	gss_node_idx target_node = gss_info->gss_node_idx;
	gss_node_idx gss_node = { .rule = rule, .input_idx = input_idx };
	

	//add a new gss_node if empty
	if(!gss[gss_idx]) {

#ifdef DEBUG
		printf("creating GSS node: (%s, %d)\n", rule_info->rule_arr.rules[rule].name, input_idx);
#endif
		//malloc the memory for the edge array
		gss[gss_idx] = init_gss_node(init_gss_edge_arrey_size, init_u_arrey_size);
		gss_edge* edge_arr = GET_GSS_EDGE_ARR(gss[gss_idx]);


		//add a new edge
		edge_arr[0].target_node = target_node;
		edge_arr[0].rule = rule_info->rule;
		edge_arr[0].alternative_start_idx = rule_info->alternative_start_idx;
		edge_arr[0].alternative_end_idx = rule_info->alternative_end_idx;
		edge_arr[0].label_type = label_type;
		
		gss[gss_idx]->edge_size = 1;

	} else { // Hence this node already exists
#ifdef DEBUG
		printf("found existing GSS node: (%s, %d)\n", rule_info->rule_arr.rules[rule].name, input_idx);
#endif
		//check if there exists an edge from gss_nodes[idx_node] to gss_node_idx
		uint32_t idx_edge;
		gss_edge* edge_arr = GET_GSS_EDGE_ARR(gss[gss_idx]);
		uint32_t edge_size = gss[gss_idx]->edge_size;
		for(idx_edge = 0; idx_edge < gss[gss_idx]->edge_size; idx_edge++) {
			if(
					edge_arr[idx_edge].target_node.input_idx == target_node.input_idx &&
					edge_arr[idx_edge].target_node.rule == target_node.rule &&
					edge_arr[idx_edge].rule == rule_info->rule &&
					edge_arr[idx_edge].alternative_start_idx == rule_info->alternative_start_idx &&
					edge_arr[idx_edge].label_type == label_type
				) break;
		}

		// no edge found
		if(idx_edge == gss[gss_idx]->edge_size) {
			//realloc the gss_edge array if its to large since we are about to add a new edge
			if(edge_size >= gss[gss_idx]->edge_alloc_size) {
				gss[gss_idx] = realloc_gss_node_edge_array(gss[gss_idx]);
			}
			gss_edge* edge_arr = GET_GSS_EDGE_ARR(gss[gss_idx]);
			uint32_t edge_size = gss[gss_idx]->edge_size;

			//add a new edge
			edge_arr[edge_size].target_node = target_node;
			edge_arr[edge_size].rule = rule_info->rule;
			edge_arr[edge_size].alternative_start_idx = rule_info->alternative_start_idx;
			edge_arr[edge_size].alternative_end_idx = rule_info->alternative_end_idx;
			edge_arr[edge_size].label_type = label_type;
			
			gss[gss_idx]->edge_size += 1;
			add_descriptor_for_P_set(rule_info, input_info, gss_node, idx_edge, gss_info, set_info);
		}
	}

	return gss_node;
}

uint32_t pop(
		struct rule_info* rule_info,
		const struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(set_info);
	assert(set_info->R_set);

	if(gss_info->gss_node_idx.rule == 92) return 1;

#ifdef DEBUG
	printf("popping GSS node\n");
#endif

	if(add_p_set_entry(set_info, gss_info, rule_info, input_info, input_info->input_idx)) return 1;

	
	uint64_t gss_idx = GET_GSS_IDX(gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_info->input_size);
	uint32_t edge_size = gss_info->gss[gss_idx]->edge_size;
	gss_edge* edge_arr = GET_GSS_EDGE_ARR(gss_info->gss[gss_idx]);
	assert(edge_arr);

	for(int i = 0; i < edge_size; i++) {
		gss_edge curr_edge = edge_arr[i];
		rule_info->rule = curr_edge.rule;
		rule_info->alternative_start_idx = curr_edge.alternative_start_idx;
		rule_info->alternative_end_idx = curr_edge.alternative_end_idx;

		gss_info->gss_node_idx = curr_edge.target_node;

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

gss_node** init_gss(uint16_t rule_count, uint32_t input_size) {
	gss_node** arr = calloc(GET_GSS_SIZE(rule_count, input_size), sizeof(gss_node*));
	assert(arr);
	return arr;
}

gss_node* init_gss_node(uint16_t edge_arr_size, uint16_t u_size) {
	gss_node* node = malloc(sizeof(gss_node) + u_size * sizeof(u_descriptors) + edge_arr_size * sizeof(gss_edge));
	assert(node);
	//place the u_set below the struct in memory
	node->u_alloc_size = u_size;
	node->u_size = 0;
	node->u_lower_idx = u_size >> 1;
	node->u_higher_idx = u_size >> 1;

	//place the edge arr below the u_set in memory
	node->edge_alloc_size = edge_arr_size;
	node->edge_size = 0;

	//init p set values to avoid uninitialized access
	node->p_count = 0;

	return node;
}

gss_node* realloc_gss_node_edge_array(gss_node* node) {
	assert(node);
	uint32_t temp_edge_size = node->edge_size;
	uint32_t temp_edge_alloc_size = node->edge_alloc_size * 2;
	gss_node* buff = realloc(node, sizeof(gss_node) + node->u_alloc_size * sizeof(u_descriptors) + temp_edge_alloc_size * sizeof(gss_edge));
	assert(buff);

	//reset the array pointers to the new position as else they may point to invalid freed data
	buff->edge_alloc_size = temp_edge_alloc_size;
	buff->edge_size = temp_edge_size;
	return buff;
}

gss_node* realloc_gss_node_u_set(gss_node* node) {
	assert(node);
	u_descriptors* old_U_set = GET_GSS_USET(node);
	gss_edge* old_edge_arr = GET_GSS_EDGE_ARR(node);

	gss_node* buff = malloc(sizeof(gss_node) + node->u_alloc_size * 2 * sizeof(u_descriptors) + node->edge_alloc_size * sizeof(gss_edge));
	assert(buff);

	buff->u_alloc_size = node->u_alloc_size * 2;
	u_descriptors* new_U_set = GET_GSS_USET(buff);
	buff->u_size = node->u_size;

	gss_edge* new_edge_arr = GET_GSS_EDGE_ARR(buff);
	buff->edge_size = node->edge_size;
	buff->edge_alloc_size = node->edge_alloc_size;

	buff->p_count = node->p_count;
	buff->p_entries[0] = node->p_entries[0];
	buff->p_entries[1] = node->p_entries[1];
	
	//i indexes the 'first' entry in the new U set we set it to half the old size since when the entire data is then copied into the new U_set it will not include a memory address jump
	uint16_t i = node->u_alloc_size >> 1;
	uint16_t j = node->u_lower_idx;
	//the first set element needs to be unrolled to avoid triggering the while condition instandly
	new_U_set[i++] = old_U_set[j];
	j = (j + 1) % node->u_alloc_size;
	while(j != node->u_higher_idx) {
		new_U_set[i++] = old_U_set[j];
		j = (j + 1) % node->u_alloc_size;
	}

	for(uint32_t k = 0; k < node->edge_size; k++) {
		new_edge_arr[k] = old_edge_arr[k];
	}

	buff->u_lower_idx = node->u_alloc_size >> 1;
	buff->u_higher_idx = i;
	free(node);

	return buff;
}

uint64_t get_gss_node_count(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t nodes = 0;
	uint64_t size = GET_GSS_SIZE(rule_count, input_size);
	for(int i = 0; i < size; i++) {
		if(!gss_info->gss[i]) continue;
		nodes += 1;
	}
	return nodes;
}

uint64_t get_gss_edge_count(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t edges = 0;
	uint64_t size = GET_GSS_SIZE(rule_count, input_size);
	for(int i = 0; i < size; i++) {
		if(!gss_info->gss[i]) continue;
		edges += gss_info->gss[i]->edge_size;
	}
	return edges;
}

uint64_t get_gss_total_alloc_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t alloc_size = 0;
	uint64_t size = GET_GSS_SIZE(rule_count, input_size);
	for(int i = 0; i < size; i++) {
		if(!gss_info->gss[i]) continue;
		alloc_size += sizeof(gss_node);
		alloc_size += gss_info->gss[i]->edge_alloc_size * sizeof(gss_edge);
		alloc_size += gss_info->gss[i]->u_alloc_size * sizeof(u_descriptors);
	}
	return alloc_size + size * sizeof(gss_node*);
}

uint64_t get_u_set_total_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t size = 0;
	uint64_t size_total = GET_GSS_SIZE(rule_count, input_size);
	for(int i = 0; i < size_total; i++) {
		if(!gss_info->gss[i]) continue;
		size += gss_info->gss[i]->u_size;
	}
	return size;
}

uint64_t get_p_set_total_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t size = 0;
	uint64_t size_total = GET_GSS_SIZE(rule_count, input_size);
	for(int i = 0; i < size_total; i++) {
		if(!gss_info->gss[i]) continue;
		size += gss_info->gss[i]->p_count;
	}
	return size;
}

int free_gss(gss_node** gss, const uint32_t rule_count, const uint32_t input_size) {
	if(!gss) return 1;
	uint64_t size_total = GET_GSS_SIZE(rule_count, input_size);
	for(int i = 0; i < size_total; i++) {
		if(!gss[i]) continue;
		free(gss[i]);
		gss[i] = NULL;
	}
	free(gss);
	return 0;
}
