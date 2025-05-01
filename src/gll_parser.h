#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include <stdint.h>

struct rule_info {
	rule* rules;
	uint16_t start_idx;
	uint16_t end_idx;
	char rule;
};

struct input_info {
	char* input;
	uint32_t input_idx;
};

struct gss_info {
	gss_node* gss_nodes;
	gss_edge* gss_edges;
	uint16_t gss_node_idx;
};

struct set_info {
	descriptors* R_set;
	descriptors* U_set;
	p_set_entry* P_set;
};

void continue_production(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		);


int start_new_production(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		);

// sets up the rule by adding each alternative to R
void init_rule(
		struct rule_info* rule_info, //start_idx and end_idx will be ignored
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		);

int base_loop(
		struct rule_info* rule_info, //rule, start_idx and end_idx will be ignored
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		);
