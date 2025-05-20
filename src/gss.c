
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
int print_gss_info(rule rules[], struct gss_info* gss_info, struct input_info* input_info, const uint8_t rule_count) {
	printf("gss (%c, %d):\n", gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx);
	for(int j = 0; j < rule_count + 2; j++) {
		for(int i = 0; i < input_info->input_size + 1; i++) {
			if(!gss_info->gss[j * (input_info->input_size + 1) + i].edge_arr) continue;

			unsigned char r = (unsigned char) 255;
			for(int n = 0; n < 28; n++) {
				if(rules[n].name != n + 'A') continue;
				if(rules[n].count_idx == j) r = n + 'A';
			}
			assert(r != 255);

			gss_node g = gss_info->gss[j * (input_info->input_size + 1) + i];
			printf("{ '%c', %d, <", r, i);
			if(g.edge_size == 0) {
				printf("> }\n");
				continue;
			}
			for(int n = 0; n < g.edge_size; n++) {
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
		struct rule_info* rule_info,
		struct input_info* input_info,
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

	gss_node* gss = gss_info->gss;
	uint32_t input_size = input_info->input_size;
	uint32_t input_idx = input_info->input_idx;
	char rule = rule_info->rule;
	uint64_t gss_idx = GET_GSS_IDX(rule_info->rules, rule, input_idx, input_size);
	gss_node_idx target_node = gss_info->gss_node_idx;
	gss_node_idx gss_node = { .rule = rule, .input_idx = input_idx };
	
#ifdef DEBUG
	printf("creating GSS node: (%c, %d)\n", rule, input_idx);
#endif

	//add a new gss_node if empty
	if(!gss[gss_idx].edge_arr) {

		//malloc the memory for the edge array
		gss[gss_idx].edge_arr = malloc(init_gss_edge_arrey_size * sizeof(gss_edge));
		gss[gss_idx].edge_alloc_size = init_gss_edge_arrey_size;

		const uint32_t init_u_arrey_size = 32;

		if(!gss[gss_idx].U_set) {
			gss[gss_idx].U_set = init_u_descriptor_set(init_u_arrey_size);
			gss[gss_idx].u_size = 0;
			gss[gss_idx].u_lower_idx = init_u_arrey_size >> 1;
			gss[gss_idx].u_higher_idx = init_u_arrey_size >> 1;
			gss[gss_idx].u_alloc_size = init_u_arrey_size;
		}

		//add a new edge
		gss[gss_idx].edge_arr[0].target_node = target_node;
		gss[gss_idx].edge_arr[0].rule = rule_info->rule;
		gss[gss_idx].edge_arr[0].alternative_start_idx = rule_info->alternative_start_idx;
		gss[gss_idx].edge_arr[0].alternative_end_idx = rule_info->alternative_end_idx;
		gss[gss_idx].edge_arr[0].label_type = label_type;
		
		gss[gss_idx].edge_size = 1;
	} else { // Hence this node already exists
		//check if there exists an edge from gss_nodes[idx_node] to gss_node_idx
		uint32_t idx_edge;
		for(idx_edge = 0; idx_edge < gss[gss_idx].edge_size; idx_edge++) {
			if(
					gss[gss_idx].edge_arr[idx_edge].target_node.input_idx == target_node.input_idx &&
					gss[gss_idx].edge_arr[idx_edge].target_node.rule == target_node.rule &&
					gss[gss_idx].edge_arr[idx_edge].rule == rule_info->rule &&
					gss[gss_idx].edge_arr[idx_edge].alternative_start_idx == rule_info->alternative_start_idx &&
					gss[gss_idx].edge_arr[idx_edge].label_type == label_type
				) break;
		}

		// no edge found
		if(idx_edge == gss[gss_idx].edge_size) {
			//realloc the gss_edge array if its to large since we are about to add a new edge
			if(gss[gss_idx].edge_size >= gss[gss_idx].edge_alloc_size) {
				gss[gss_idx].edge_alloc_size *= 2;
				gss[gss_idx].edge_arr = realloc(gss[gss_idx].edge_arr, gss[gss_idx].edge_alloc_size * sizeof(gss_edge));
				assert(gss[gss_idx].edge_arr);
			}

			//add a new edge
			gss[gss_idx].edge_arr[gss[gss_idx].edge_size].target_node = target_node;
			gss[gss_idx].edge_arr[gss[gss_idx].edge_size].rule = rule_info->rule;
			gss[gss_idx].edge_arr[gss[gss_idx].edge_size].alternative_start_idx = rule_info->alternative_start_idx;
			gss[gss_idx].edge_arr[gss[gss_idx].edge_size].alternative_end_idx = rule_info->alternative_end_idx;
			gss[gss_idx].edge_arr[gss[gss_idx].edge_size].label_type = label_type;
			
			gss[gss_idx].edge_size += 1;
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

	
	uint64_t gss_idx = GET_GSS_IDX(rule_info->rules, gss_info->gss_node_idx.rule, gss_info->gss_node_idx.input_idx, input_info->input_size);
	gss_node gss_node = gss_info->gss[gss_idx];
	assert(gss_node.edge_arr);

	for(int i = 0; i < gss_node.edge_size; i++) {
		gss_edge curr_edge = gss_node.edge_arr[i];
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
		edges += gss_info->gss[i].edge_size;
	}
	return edges;
}

uint64_t get_gss_total_alloc_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t alloc_size = 0;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		if(gss_info->gss[i].edge_arr) 
			alloc_size += gss_info->gss[i].edge_alloc_size * sizeof(gss_edge);
		if(gss_info->gss[i].U_set) 
			alloc_size += gss_info->gss[i].u_alloc_size * sizeof(u_descriptors);
	}
	return alloc_size + GET_GSS_SIZE(rule_count, input_size) * sizeof(gss_node);
}

uint64_t get_u_set_total_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t size = 0;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		if(!gss_info->gss[i].U_set) continue;
		size += gss_info->gss[i].u_size;
	}
	return size;
}

uint64_t get_p_set_total_size(const struct gss_info* gss_info, uint32_t rule_count, uint32_t input_size) {
	uint64_t size = 0;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		size += gss_info->gss[i].p_count;
	}
	return size;

}

int free_gss(gss_node* gss, const uint32_t rule_count, const uint32_t input_size) {
	if(!gss) return 1;
	for(int i = 0; i < GET_GSS_SIZE(rule_count, input_size); i++) {
		if(gss[i].edge_arr) free(gss[i].edge_arr);
		if(gss[i].U_set) free(gss[i].U_set);
		gss[i].edge_arr = NULL;
	}
	free(gss);
	return 0;
}
