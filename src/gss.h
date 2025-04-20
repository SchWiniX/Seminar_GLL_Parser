#ifndef GSS
#define GSS

#include "binarized_sppf.h"

const uint16_t GSS_NODE_LIST_SIZE = 256;
const uint16_t GSS_EDGE_LIST_SIZE = 256;

typedef struct gss_edge {
	uint16_t src_node;
	uint16_t target_node;
	uint16_t record;
} gss_edge;

typedef struct gss_node{
	uint16_t* outgoing_edges;
	char* grammar_slot;
	uint32_t in_pos;
	uint16_t edge_list_size;
	uint8_t grammar_slot_idx;
} gss_node;

gss_node* create(gss_node start_node[], const char* grammar_slot, int grammar_slot_pos, int in_pos, sppf_node* record);

int pop(gss_node* c_u, int c_i, sppf_node* c_n);

#endif
