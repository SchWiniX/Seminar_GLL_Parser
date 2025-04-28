#ifndef GSS
#define GSS

#include "descriptor_set_functions.h"

extern uint16_t gss_node_total_array_size;
extern uint16_t gss_edge_total_array_size;

typedef struct gss_edge {
	uint16_t src_node;
	uint16_t target_node;
	uint16_t record;
} gss_edge;

typedef struct gss_node{
	uint16_t grammar_rule_idx;
	uint16_t grammar_slot_idx;
	uint16_t grammar_slot_pos;
	uint16_t in_pos;
} gss_node;

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
		);

int pop(
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		uint16_t c_u,
		int c_i
		);

#endif
