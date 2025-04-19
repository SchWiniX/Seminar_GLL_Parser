#ifndef GSS
#define GSS

#include "binarized_sppf.h"

typedef struct gss_edge {
	struct gss_node* src_node;
	struct gss_node* target_node;
	sppf_node* record;
} gss_edge;

typedef struct gss_node{
	char* grammar_slot;
	int grammar_slot_pos;
	int in_pos;
	gss_edge** outgoing_edges;
	int edge_list_size;
} gss_node;

gss_node* create(gss_node* start_node, const char* grammar_slot, int grammar_slot_pos, int in_pos, sppf_node* record);

int pop(gss_node* c_u, int c_i, sppf_node* c_n);

#endif
