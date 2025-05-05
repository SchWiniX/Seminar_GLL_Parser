#ifndef GSS
#define GSS

#include "info_struct.h"

enum LABEL_TYPE {PARTIAL_PRODUCTION, FULL_PRODUCTION, RULE, BASELOOP, INVALID, SIZE};

int print_gss_info(rule rules[], struct gss_info* gss_info);

uint32_t create(
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info,
		uint8_t label_type
		);

int pop(
		const struct input_info* input_info,
		const struct gss_info* gss_info,
		struct set_info* set_info
		);

gss_node* init_node_array(uint32_t size);
gss_edge* init_edge_array(uint32_t size);

int free_gss(gss_node* gss_nodes, gss_edge* gss_edges);

#endif
