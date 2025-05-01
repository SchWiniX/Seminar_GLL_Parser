#ifndef GSS
#define GSS

#include "descriptor_set_functions.h"

enum LABEL_TYPE {EMPTY, DO_SINGLE_PARSES, DO_PRODUCTION, DO_RULE, SIZE};

extern uint16_t gss_node_alloc_array_size;
extern uint16_t gss_edge_alloc_array_size;

typedef struct gss_edge {
	uint16_t src_node;
	uint16_t target_node;
} gss_edge;

typedef struct gss_node{
	uint16_t rule;
	uint16_t block_idx;
	uint16_t block_end_idx;
	uint32_t input_idx;
	uint8_t label_type;
} gss_node;

uint16_t create(
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		uint32_t input_idx,
		uint16_t rule,
		uint16_t block_idx,
		uint16_t block_end_idx,
		uint16_t gss_node_idx,
		uint8_t label_type
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

gss_node* init_node_array();
gss_edge* init_edge_array();

#endif
