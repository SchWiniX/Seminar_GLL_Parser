#ifndef GLLPARSER
#define GLLPARSER

#include <stdint.h>
#include "descriptor_set_functions.h"
#include "grammer_handler.h"

enum LABEL_TYPE {PARTIAL_ALTERNATIVE, WHOLE_ALTERNATIVE, RULE, BASELOOP, INVALID, SIZE};

int print_input_info(struct input_info* input_info);

void continue_alternative(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		);


void start_new_alternative(
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

#endif
