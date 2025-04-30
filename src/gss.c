
#include<stdint.h>
#include<stdlib.h>
#include<assert.h>
#include"gss.h"
#include "descriptor_set_functions.h"

uint16_t gss_node_alloc_array_size = 256;
uint16_t gss_edge_alloc_array_size = 256;
uint16_t gss_node_array_size = 0;
uint16_t gss_edge_array_size = 0;

uint16_t create(
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		uint16_t rule,
		uint16_t block_idx,
		uint16_t input_idx,
		uint16_t gss_node_idx,
		uint8_t do_exec_label
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
		gss_nodes[gss_node_array_size].input_idx = input_idx;
		gss_nodes[gss_node_array_size].do_exec_label = do_exec_label;

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
					P_set[i].input_idx,
					idx_node
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

	for(int i = 0; i < gss_edge_array_size; i++) {
		if(gss_edges[i].src_node == c_u) {
			gss_node curr_node = gss_nodes[gss_edges[i].target_node];
			add_descriptor(
					R_set,
					U_set,
					curr_node.rule,
					curr_node.block_idx,
					c_i,
					c_u
					);
		}
	}
	add_p_set_entry(P_set, c_u, c_i);
	return 0;
}
