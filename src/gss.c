
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "gss.h"
#include "descriptor_set_functions.h"
#include "info_struct.h"
#include "debug.h"

#ifdef DEBUG
int print_gss_info(rule rules[], struct gss_info* gss_info) {

	printf("gss_nodes: [%d:%d] { ", gss_info->gss_node_array_size, gss_info->gss_node_alloc_array_size);
	if(gss_info->gss_node_array_size == 0) {
		printf("}\n");
		goto EDGES;
	}
	printf("(%d: %c, [%d[", 0, gss_info->gss_nodes[0].rule, gss_info->gss_nodes[0].block_idx);
	for(int j = gss_info->gss_nodes[0].block_idx; j < gss_info->gss_nodes[0].block_end_idx; j++) {
		printf("%c", rules[gss_info->gss_nodes[0].rule - 'A'].blocks[j]);
	}
	printf("]%d], %d, %d)", gss_info->gss_nodes[0].block_end_idx, gss_info->gss_nodes[0].input_idx, gss_info->gss_nodes[0].label_type);

	for(int i = 1; i < gss_info->gss_node_array_size; i++) {
		printf(", (%d: %c, [%d[", i, gss_info->gss_nodes[i].rule, gss_info->gss_nodes[i].block_idx);
		for(int j = gss_info->gss_nodes[i].block_idx; j < gss_info->gss_nodes[i].block_end_idx; j++) {
			printf("%c", rules[gss_info->gss_nodes[i].rule - 'A'].blocks[j]);
		}
		printf("]%d], %d, %d)", gss_info->gss_nodes[i].block_end_idx, gss_info->gss_nodes[i].input_idx, gss_info->gss_nodes[i].label_type);
	}
	printf(" }, gss_node_idx: %d\n", gss_info->gss_node_idx);

EDGES:

	printf("gss_edges: [%d:%d] { ", gss_info->gss_edge_array_size, gss_info->gss_edge_alloc_array_size);
	if(gss_info->gss_edge_array_size == 0) {
		printf("}\n");
		return 0;
	}
	printf("(%d -> %d)", gss_info->gss_edges[0].src_node, gss_info->gss_edges[0].target_node);
	for(int i = 1; i < gss_info->gss_edge_array_size; i++) {
		printf(", (%d -> %d)", gss_info->gss_edges[i].src_node, gss_info->gss_edges[i].target_node);
	}
	printf(" }\n");

	return 0;
}
#endif
 
uint32_t create(
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
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	//check if there is already a gss_node with these values
	uint32_t idx_node;
	for(idx_node = 0; idx_node < gss_info->gss_node_array_size; idx_node++){
		if(
				gss_info->gss_nodes[idx_node].rule == rule_info->rule &&
				gss_info->gss_nodes[idx_node].block_idx == rule_info->start_idx &&
				gss_info->gss_nodes[idx_node].input_idx == input_info->input_idx
			) break;
	}

	//add a new gss_node
	if(idx_node == gss_info->gss_node_array_size) {
		//Realloc the gss_node array if its to large since we are about to add a new node
		if(gss_info->gss_node_array_size >= gss_info->gss_node_alloc_array_size) {
			gss_info->gss_node_alloc_array_size *= 2;
			gss_info->gss_nodes = (gss_node*) realloc(gss_info->gss_nodes, gss_info->gss_node_alloc_array_size * sizeof(gss_node));
			assert(gss_info->gss_nodes);
		}

		gss_info->gss_nodes[gss_info->gss_node_array_size].rule = rule_info->rule;
		gss_info->gss_nodes[gss_info->gss_node_array_size].block_idx = rule_info->start_idx;
		gss_info->gss_nodes[gss_info->gss_node_array_size].block_end_idx = rule_info->end_idx;
		gss_info->gss_nodes[gss_info->gss_node_array_size].input_idx = input_info->input_idx;
		gss_info->gss_nodes[gss_info->gss_node_array_size].label_type = label_type;

	}

	//check if there exists an edge from gss_nodes[idx_node] to c_n
	uint32_t idx_edge;
	for(idx_edge = 0; idx_edge < gss_info->gss_edge_array_size; idx_edge++) {
		if(
				gss_info->gss_edges[idx_edge].src_node == idx_node &&
				gss_info->gss_edges[idx_edge].target_node == gss_info->gss_node_idx
			) break;
	}

	if(idx_edge == gss_info->gss_edge_array_size) {
		//Realloc the gss_edge array if its to large since we are about to add a new edge
		if(gss_info->gss_edge_array_size >= gss_info->gss_edge_alloc_array_size) {
			gss_info->gss_edge_alloc_array_size *= 2;
			gss_info->gss_edges = (gss_edge*) realloc(gss_info->gss_edges, gss_info->gss_edge_alloc_array_size * sizeof(gss_edge));
			assert(gss_info->gss_edges);
		}

		//add a new edge
		gss_info->gss_edges[gss_info->gss_edge_array_size].src_node = idx_node;
		gss_info->gss_edges[gss_info->gss_edge_array_size].target_node = gss_info->gss_node_idx;
		
		gss_info->gss_edge_array_size += 1;
	}

	add_descriptor_for_P_set(gss_info, set_info, idx_node);

	if(idx_node == gss_info->gss_node_array_size) {//if node has been added
		gss_info->gss_node_array_size += 1;
	}

	return idx_node;
}

int pop(
		const struct input_info* input_info,
		const struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	if(gss_info->gss_node_idx == 0) return 1;

	add_p_set_entry(set_info, gss_info->gss_node_idx, input_info->input_idx);
	for(int i = 0; i < gss_info->gss_edge_array_size; i++) {
		if(gss_info->gss_edges[i].src_node == gss_info->gss_node_idx) {
			gss_node curr_node = gss_info->gss_nodes[gss_info->gss_node_idx];
			struct rule_info r = {
				.rules = NULL,
				.rule = curr_node.rule,
				.start_idx = curr_node.block_idx,
				.end_idx = curr_node.block_end_idx,
			};
			add_descriptor(
					&r,
					set_info,
					input_info->input_idx,
					gss_info->gss_edges[i].target_node,
					curr_node.label_type
					);
		}
	}
	return 0;
}

gss_node* init_node_array(uint32_t size) {
	gss_node* arr = (gss_node*) malloc(size * sizeof(gss_node));
	assert(arr);
	return arr;
}
gss_edge* init_edge_array(uint32_t size) {
	gss_edge* arr = (gss_edge*) malloc(size * sizeof(gss_edge));
	assert(arr);
	return arr;
}

int free_gss(gss_node* gss_nodes, gss_edge* gss_edges) {
	if(!gss_nodes)
		free(gss_nodes);
	if(!gss_edges)
		free(gss_edges);
	return 0;	
}
