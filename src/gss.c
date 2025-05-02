
#include <stddef.h>
#include<stdint.h>
#include<stdlib.h>
#include<assert.h>
#include"gss.h"
#include "descriptor_set_functions.h"

uint16_t gss_node_alloc_array_size = 256;
uint16_t gss_edge_alloc_array_size = 256;
uint16_t gss_node_array_size = 0;
uint16_t gss_edge_array_size = 0;

int print_gss_info(rule rules[], struct gss_info* gss_info) {

	printf("gss_nodes: [%d:%d] { ", gss_node_array_size, gss_node_alloc_array_size);
	if(gss_node_array_size == 0) {
		printf("}\n");
		goto EDGES;
	}
	printf("(%d: %c, [%d[", 0, gss_info->gss_nodes[0].rule, gss_info->gss_nodes[0].block_idx);
	for(int j = gss_info->gss_nodes[0].block_idx; j < gss_info->gss_nodes[0].block_end_idx; j++) {
		printf("%c", rules[gss_info->gss_nodes[0].rule - 65].blocks[j]);
	}
	printf("]%d], %d, %d)", gss_info->gss_nodes[0].block_end_idx, gss_info->gss_nodes[0].input_idx, gss_info->gss_nodes[0].label_type);

	for(int i = 1; i < gss_node_array_size; i++) {
		printf(", (%d: %c, [%d[", i, gss_info->gss_nodes[i].rule, gss_info->gss_nodes[i].block_idx);
		for(int j = gss_info->gss_nodes[i].block_idx; j < gss_info->gss_nodes[i].block_end_idx; j++) {
			printf("%c", rules[gss_info->gss_nodes[i].rule - 65].blocks[j]);
		}
		printf("]%d], %d, %d)", gss_info->gss_nodes[i].block_end_idx, gss_info->gss_nodes[i].input_idx, gss_info->gss_nodes[i].label_type);
	}
	printf(" }, gss_node_idx: %d\n", gss_info->gss_node_idx);

EDGES:

	printf("gss_edges: [%d:%d] { ", gss_edge_array_size, gss_edge_alloc_array_size);
	if(gss_edge_array_size == 0) {
		printf("}\n");
		return 0;
	}
	printf("(%d -> %d)", gss_info->gss_edges[0].src_node, gss_info->gss_edges[0].target_node);
	for(int i = 1; i < gss_edge_array_size; i++) {
		printf(", (%d -> %d)", gss_info->gss_edges[i].src_node, gss_info->gss_edges[i].target_node);
	}
	printf(" }\n");

	return 0;
}

uint16_t create(
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		uint32_t input_idx,
		uint16_t block_idx,
		uint16_t block_end_idx,
		uint16_t gss_node_idx,
		uint8_t label_type,
		char rule
		) {

	assert(gss_nodes);	
	assert(gss_edges);	
	assert(R_set);
	assert(U_set);
	assert(P_set);
	//check if there is already a gss_node with these values
	uint16_t idx_node;
	for(idx_node = 0; idx_node < gss_node_array_size; idx_node++){
		if(
				gss_nodes[idx_node].rule == rule &&
				gss_nodes[idx_node].block_idx == block_idx &&
				gss_nodes[idx_node].input_idx == input_idx
			) break;
	}

	//add a new gss_node
	if(idx_node == gss_node_array_size) {
		//Realloc the gss_node array if its to large since we are about to add a new node
		if(gss_node_array_size >= gss_node_alloc_array_size) {
			gss_node_alloc_array_size *= 2;
			gss_nodes = (gss_node*) realloc(gss_nodes, gss_node_alloc_array_size * sizeof(gss_node));
			assert(gss_nodes);
		}

		gss_nodes[gss_node_array_size].rule = rule;
		gss_nodes[gss_node_array_size].block_idx = block_idx;
		gss_nodes[gss_node_array_size].block_end_idx = block_end_idx;
		gss_nodes[gss_node_array_size].input_idx = input_idx;
		gss_nodes[gss_node_array_size].label_type = label_type;

	}

	//check if there exists an edge from gss_nodes[idx_node] to c_n
	uint16_t idx_edge;
	for(idx_edge = 0; idx_edge < gss_edge_array_size; idx_edge++) {
		if(
				gss_edges[idx_edge].src_node == idx_node &&
				gss_edges[idx_edge].target_node == gss_node_idx
			) break;
	}

	if(idx_edge == gss_edge_array_size) {
		//Realloc the gss_edge array if its to large since we are about to add a new edge
		if(gss_edge_array_size >= gss_edge_alloc_array_size) {
			gss_edge_alloc_array_size *= 2;
			gss_edges = (gss_edge*) realloc(gss_edges, gss_edge_alloc_array_size * sizeof(gss_edge));
			assert(gss_edges);
		}

		//add a new edge
		gss_edges[gss_edge_array_size].src_node = idx_node;
		gss_edges[gss_edge_array_size].target_node = gss_node_idx;
		
		gss_edge_array_size += 1;
	}

	//add if in P
	int i;
	for(i = 0; i < p_size; i++) {
		if(P_set[i].gss_node_idx == idx_node) {
			gss_node curr_node = gss_nodes[idx_node];
			add_descriptor(
					R_set,
					U_set,
					curr_node.rule,
					curr_node.block_idx,
					curr_node.block_end_idx,
					P_set[i].input_idx,
					idx_node,
					curr_node.label_type
					);
			break;
		}
	}
	if(idx_node == gss_node_array_size) {//if node has been found and retrieved
		gss_node_array_size += 1;
	}

	return idx_node;
}

int pop(
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		uint16_t gss_node_idx,
		int input_idx
		) {

	assert(gss_nodes);
	assert(gss_edges);
	assert(R_set);
	assert(U_set);
	assert(P_set);

	if(gss_node_idx == 0) return 1;

	for(int i = 0; i < gss_edge_array_size; i++) {
		if(gss_edges[i].src_node == gss_node_idx) {
			gss_node curr_node = gss_nodes[gss_edges[i].target_node];
			printf("Found gss edge %d -> %d, so add descriptor (%c, [%d, %d], %d, %d, %d)\n", gss_node_idx, gss_edges[i].target_node, curr_node.rule, curr_node.block_idx, curr_node.block_end_idx, input_idx, gss_node_idx, curr_node.label_type);
			add_descriptor(
					R_set,
					U_set,
					curr_node.rule,
					curr_node.block_idx,
					curr_node.block_end_idx,
					input_idx,
					gss_node_idx,
					curr_node.label_type
					);
		}
	}
	add_p_set_entry(P_set, gss_node_idx, input_idx);
	return 0;
}

gss_node* init_node_array() {
	gss_node* arr = (gss_node*) malloc(gss_node_alloc_array_size * sizeof(gss_node));
	assert(arr);
	return arr;
}
gss_edge* init_edge_array() {
	gss_edge* arr = (gss_edge*) malloc(gss_node_alloc_array_size * sizeof(gss_edge));
	assert(arr);
	return arr;
}
