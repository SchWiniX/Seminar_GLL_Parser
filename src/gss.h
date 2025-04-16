#ifndef GSS
#define GSS

#include "binarized_sppf.h"

typedef struct {
	struct gss_node* src_node;
	struct gss_node* target_node;
	sppf_node* record;
} gss_edge;

typedef struct gss_node{
	char* grammar_slot;
	int grammar_slot_pos;
	int in_pos;
} gss_node;



#endif
