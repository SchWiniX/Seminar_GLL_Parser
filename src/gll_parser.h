#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include <stdint.h>

struct input_info {
	char* input;
	uint32_t input_idx;
	uint32_t input_size;
};

int print_input_info(struct input_info* input_info);

void continue_production(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		);


void start_new_production(
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
