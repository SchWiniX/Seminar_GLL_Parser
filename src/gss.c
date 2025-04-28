
#include<stdint.h>
#include<stdlib.h>
#include<assert.h>
#include"gss.h"
#include "descriptor_set_functions.h"

uint16_t gss_node_total_array_size = 256;
uint16_t gss_edge_total_array_size = 256;
uint16_t gss_node_array_size = 0;
uint16_t gss_edge_array_size = 0;

uint16_t create(
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_pos,
		uint16_t grammar_slot_idx,
		uint16_t in_pos,
		uint16_t c_n,
		uint16_t c_u
		) {

	assert(gss_nodes);	
	//check if there is already a gss_node with these values
	uint16_t idx_node;
	for(idx_node = 0; idx_node < gss_node_array_size; idx_node++){
		if(
				gss_nodes[idx_node].grammar_rule_idx == grammar_rule_idx &&
				gss_nodes[idx_node].grammar_slot_idx == grammar_slot_idx &&
				gss_nodes[idx_node].grammar_slot_pos == grammar_slot_pos &&
				gss_nodes[idx_node].in_pos == in_pos
			) break;
	}

	if(idx_node == gss_node_array_size) {
		//Realloc the gss_node array if its to large since we are about to add a new node
		if(gss_node_array_size >= gss_node_total_array_size) {
			gss_node_total_array_size *= 2;
			gss_nodes = (gss_node*) realloc(gss_nodes, gss_node_total_array_size * sizeof(gss_node));
			assert(gss_nodes);
		}

		//add a new gss_node
		gss_nodes[gss_node_array_size].grammar_rule_idx = grammar_rule_idx = grammar_rule_idx;
		gss_nodes[gss_node_array_size].grammar_slot_idx = grammar_slot_idx;
		gss_nodes[gss_node_array_size].grammar_slot_pos = grammar_slot_pos;
		gss_nodes[gss_node_array_size].in_pos = in_pos;

	}

	//check if there exists an edge from gss_nodes[idx_node] to c_n
	uint16_t idx_edge;
	for(idx_edge = 0; idx_edge < gss_edge_array_size; idx_edge++) {
		if(
				gss_edges[idx_edge].src_node == idx_node &&
				gss_edges[idx_edge].target_node == c_n &&
				gss_edges[idx_edge].record == c_u
			) break;
	}

	if(idx_edge == gss_edge_array_size) {
		//Realloc the gss_edge array if its to large since we are about to add a new edge
		if(gss_edge_array_size >= gss_edge_total_array_size) {
			gss_edge_total_array_size *= 2;
			gss_edges = (gss_edge*) realloc(gss_edges, gss_edge_total_array_size * sizeof(gss_edge));
			assert(gss_edges);
		}

		//add a new edge
		gss_edges[gss_edge_array_size].src_node = idx_node;
		gss_edges[gss_edge_array_size].target_node = c_n;
		gss_edges[gss_edge_array_size].record = c_u;
		
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
					curr_node.grammar_rule_idx,
					curr_node.grammar_slot_idx,
					curr_node.grammar_slot_pos,
					P_set[i].in_pos,
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
		uint16_t c_u,
		int c_i
		) {

	for(int i = 0; i < gss_edge_array_size; i++) {
		if(gss_edges[i].src_node == c_u) {
			gss_node curr_node = gss_nodes[gss_edges[i].target_node];
			add_descriptor(
					R_set,
					U_set,
					curr_node.grammar_rule_idx,
					curr_node.grammar_slot_idx,
					curr_node.grammar_slot_pos,
					c_i,
					c_u
					);
		}
	}
	add_p_set_entry(P_set, c_u, c_i);
	return 0;
}
