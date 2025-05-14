
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gss.h"
#include "descriptor_set_functions.h"
#include "info_struct.h"
#include "debug.h"

const uint32_t init_gss_edge_arrey_size = 16;

#ifdef DEBUG
int print_gss_info(rule rules[], struct gss_info* gss_info, struct input_info* input_info) {

	printf("gss:\n");
	for(int j = 0; j < 28; j++) {
		for(int i = 0; i < input_info->input_size + 1; i++) {
			if(!gss_info->gss[j * (input_info->input_size + 1) + i].edge_arr) continue;
			gss_node g = gss_info->gss[j * (input_info->input_size + 1) + i];
			printf("{ '%c', %d, <", j + 'A', i);
			if(g.size == 0) {
				printf("> }\n");
				continue;
			}
			for(int n = 0; n < g.size; n++) {
				printf("(-> (%c, %d), %c, [%d[", g.edge_arr[n].target_node.rule, g.edge_arr[n].target_node.input_idx, g.edge_arr[n].rule, g.edge_arr[n].alternative_start_idx);
				for(int m = g.edge_arr[n].alternative_start_idx; m < g.edge_arr[n].alternative_end_idx; m++) {
					printf("%c", rules[g.edge_arr[n].rule - 'A'].alternatives[m]);
				}
				printf("]%d], %d)", g.edge_arr[n].alternative_end_idx, g.edge_arr[n].label_type);
			}
			printf("> }\n");
		}
	}
	return 0;
}
#endif
 
gss_node_idx create(
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info,
		uint8_t label_type
		) {

	assert(rule_info);
	assert(rule_info->rules);
	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);



	gss_node* gss = gss_info->gss;
	uint32_t input_size = input_info->input_size;
	uint32_t input_idx = input_info->input_idx;
	char rule = rule_info->rule;
	uint32_t gss_idx = (input_size + 1) * (rule - 'A') + input_idx;
	gss_node_idx target_node = gss_info->gss_node_idx;
	gss_node_idx gss_node = { .rule = rule, .input_idx = input_idx };
	
#ifdef DEBUG
	printf("creating GSS node: (%c, %d)\n", rule, input_idx);
#endif

	//add a new gss_node if empty
	if(!gss[gss_idx].edge_arr) {

		//malloc the memory for the edge array
		gss[gss_idx].edge_arr = malloc(init_gss_edge_arrey_size * sizeof(gss_edge));
		gss[gss_idx].alloc_size = init_gss_edge_arrey_size;

		//add a new edge
		gss[gss_idx].edge_arr[0].target_node = target_node;
		gss[gss_idx].edge_arr[0].rule = rule_info->rule;
		gss[gss_idx].edge_arr[0].alternative_start_idx = rule_info->alternative_start_idx;
		gss[gss_idx].edge_arr[0].alternative_end_idx = rule_info->alternative_end_idx;
		gss[gss_idx].edge_arr[0].label_type = label_type;
		
		gss[gss_idx].size = 1;
	} else { // Hence this node already exists
		//check if there exists an edge from gss_nodes[idx_node] to gss_node_idx
		uint32_t idx_edge;
		for(idx_edge = 0; idx_edge < gss[gss_idx].size; idx_edge++) {
			if(
					gss[gss_idx].edge_arr[idx_edge].target_node.input_idx == target_node.input_idx &&
					gss[gss_idx].edge_arr[idx_edge].target_node.rule == target_node.rule &&
					gss[gss_idx].edge_arr[idx_edge].rule == rule_info->rule &&
					gss[gss_idx].edge_arr[idx_edge].alternative_start_idx == rule_info->alternative_start_idx &&
					gss[gss_idx].edge_arr[idx_edge].label_type == label_type
				) break;
		}

		// no edge found
		if(idx_edge == gss[gss_idx].size) {
			//realloc the gss_edge array if its to large since we are about to add a new edge
			if(gss[gss_idx].size >= gss[gss_idx].alloc_size) {
				gss[gss_idx].alloc_size *= 2;
				gss[gss_idx].edge_arr = realloc(gss[gss_idx].edge_arr, gss[gss_idx].alloc_size * sizeof(gss_edge));
				assert(gss[gss_idx].edge_arr);
			}

			//add a new edge
			gss[gss_idx].edge_arr[gss[gss_idx].size].target_node = target_node;
			gss[gss_idx].edge_arr[gss[gss_idx].size].rule = rule_info->rule;
			gss[gss_idx].edge_arr[gss[gss_idx].size].alternative_start_idx = rule_info->alternative_start_idx;
			gss[gss_idx].edge_arr[gss[gss_idx].size].alternative_end_idx = rule_info->alternative_end_idx;
			gss[gss_idx].edge_arr[gss[gss_idx].size].label_type = label_type;
			
			gss[gss_idx].size += 1;
			add_descriptor_for_P_set(gss_info, input_info, gss_node, idx_edge, set_info);
		}
	}

	return gss_node;
}

uint32_t pop(
		const struct input_info* input_info,
		const struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	if(gss_info->gss_node_idx.rule == 92) return 1;

#ifdef DEBUG
	printf("popping GSS node\n");
#endif

	if(add_p_set_entry(set_info, gss_info->gss_node_idx, input_info->input_idx)) return 1;

	
	char rule = gss_info->gss_node_idx.rule;
	uint32_t input_idx = gss_info->gss_node_idx.input_idx;
	uint32_t input_size = input_info->input_size;
	uint64_t gss_idx = GET_GSS_IDX(rule, input_idx, input_size);
	gss_node gss_node = gss_info->gss[gss_idx];
	assert(gss_node.edge_arr);

	for(int i = 0; i < gss_node.size; i++) {
		gss_edge curr_edge = gss_node.edge_arr[i];
		struct rule_info r = {
			.rules = NULL,
			.rule = curr_edge.rule,
			.alternative_start_idx = curr_edge.alternative_start_idx,
			.alternative_end_idx = curr_edge.alternative_end_idx,
		};
		add_descriptor(
				&r,
				set_info,
				input_info->input_idx,
				curr_edge.target_node,
				curr_edge.label_type
				);
		
	}
	return 0;
}

gss_node* init_gss(uint32_t rule_count, uint32_t input_size) {
	gss_node* arr = calloc(GET_GSS_SIZE(rule_count, input_size), sizeof(gss_node));
	assert(arr);
	return arr;
}

uint64_t get_gss_node_count(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t nodes = 0;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		if(!gss_info->gss[i].edge_arr) continue;
		nodes += 1;
	}
	return nodes;
}

uint64_t get_gss_edge_count(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t edges = 0;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		if(!gss_info->gss[i].edge_arr) continue;
		edges += gss_info->gss[i].size;
	}
	return edges;
}

uint64_t get_gss_total_alloc_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t alloc_size = 0;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		if(!gss_info->gss[i].edge_arr) continue;
		alloc_size += gss_info->gss[i].alloc_size * sizeof(gss_edge);
	}
	return alloc_size + GET_GSS_SIZE(rule_count, input_size) * sizeof(gss_node);
}

int free_gss(gss_node* gss, const uint32_t rule_count, const uint32_t input_size) {
	if(!gss) return 1;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		if(!gss[i].edge_arr) continue;
		free(gss[i].edge_arr);
		gss[i].edge_arr = NULL;
	}
	free(gss);
	return 0;
}
