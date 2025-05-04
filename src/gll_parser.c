#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>

#include "descriptor_set_functions.h"
#include "gll_parser.h"
#include "grammer_handler.h"
#include "gss.h"

jmp_buf L0_jump_buf;


int print_input_info(struct input_info* input_info) {
	printf("Input [%d, %d]: ", input_info->input_idx, input_info->input_size);
	for(int i = 0; i <= input_info->input_size; i++) {
		if(i == input_info->input_idx) {
			printf(">%c<", input_info->input[i]);
		} else {
			printf("%c", input_info->input[i]);
		}
	}
	printf("\n");
	return 0;
}

int check_success(descriptors U_set[], uint16_t u_size, uint16_t rule, uint32_t input_idx) {
	assert(U_set);
	return U_set[u_size - 1].rule == rule &&
		U_set[u_size - 1].input_idx == input_idx &&
		U_set[u_size - 1].block_idx == U_set[u_size - 1].block_end_idx;
}

#pragma GCC diagnostic ignored "-Winfinite-recursion"
void continue_production(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(rule_info);
	assert(rule_info->rules);
	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	//printf("entered continue_production\n");

	struct rule this_rule = rule_info->rules[rule_info->rule - 65];
	uint16_t start_idx = rule_info->start_idx;
	uint16_t end_idx = rule_info->end_idx;
	if(start_idx == end_idx) {
		pop(input_info, gss_info, set_info);
		longjmp(L0_jump_buf, 1); //goback to base_loop
	}
	if(!is_non_terminal(this_rule.blocks[start_idx])) {
		if(this_rule.blocks[start_idx] == input_info->input[input_info->input_idx]) {
			input_info->input_idx += 1;
			rule_info->start_idx += 1;
			continue_production(rule_info, input_info, gss_info, set_info);
		} else longjmp(L0_jump_buf, 1); //goback to base_loop
	} else {
		if(first_follow_test(rule_info, input_info->input[input_info->input_idx])) {
			rule_info->start_idx += 1;
			gss_info->gss_node_idx = create(
					rule_info,
					input_info,
					gss_info,
					set_info,
					PARTIAL_PRODUCTION
					);
			rule_info->start_idx -= 1;
			rule_info->rule = this_rule.blocks[start_idx];
			init_rule(rule_info, input_info, gss_info, set_info);
		} else longjmp(L0_jump_buf, 1); //goback to base_loop
	}
}


void start_new_production(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(rule_info);
	assert(rule_info->rules);
	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	//printf("entered start_new_production\n");

	struct rule this_rule = rule_info->rules[rule_info->rule - 65];
	uint16_t start_idx = rule_info->start_idx;
	uint16_t end_idx = rule_info->end_idx;
	if(start_idx + 1 == end_idx && this_rule.blocks[start_idx] == '_') {
		pop(input_info, gss_info, set_info);

		longjmp(L0_jump_buf, 1); //goback to base_loop
	} else if(!is_non_terminal(this_rule.blocks[start_idx])) {
		input_info->input_idx += 1;
		rule_info->start_idx += 1;
		continue_production(rule_info, input_info, gss_info, set_info);
	} else {
		rule_info->start_idx += 1;
		gss_info->gss_node_idx = create(
				rule_info,
				input_info,
				gss_info,
				set_info,
				PARTIAL_PRODUCTION
				);
		rule_info->start_idx -= 1;
		rule_info->rule = this_rule.blocks[start_idx];
		init_rule(rule_info, input_info, gss_info, set_info);
	}
	//this should be unreachable
	assert(1);
}

// sets up the rule by adding each alternative to R
void init_rule(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(rule_info);
	assert(rule_info->rules);
	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	//printf("entered init_rule for %c\n", rule_info->rule);

	struct rule this_rule = rule_info->rules[rule_info->rule - 65];
	for(int i = 0; i < this_rule.number_of_blocks; i++) {
		rule_info->start_idx = this_rule.block_sizes[i];
		rule_info->end_idx = this_rule.block_sizes[i + 1];
		if(first_follow_test(rule_info, input_info->input[input_info->input_idx])) {
			add_descriptor(rule_info, set_info, input_info->input_idx, gss_info->gss_node_idx, FULL_PRODUCTION);
		}
	}
	longjmp(L0_jump_buf, 1); //goback to base_loop
}

int base_loop(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(rule_info);
	assert(rule_info->rules);
	assert(input_info);
	assert(input_info->input);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	gss_info->gss_nodes[0].rule = '0';
	gss_info->gss_nodes[0].block_idx = 0;
	gss_info->gss_nodes[0].block_end_idx = 0;
	gss_info->gss_nodes[0].input_idx = 0;
	gss_info->gss_nodes[0].label_type = INVALID;

	gss_info->gss_nodes[1].rule = '0';
	gss_info->gss_nodes[1].block_idx = 0;
	gss_info->gss_nodes[1].block_end_idx = 0;
	gss_info->gss_nodes[1].input_idx = 0;
	gss_info->gss_nodes[1].label_type = BASELOOP;

	gss_info->gss_edges[0].src_node = 1;
	gss_info->gss_edges[0].target_node = 0;

	gss_info->gss_node_array_size = 2;
	gss_info->gss_edge_array_size = 1;
	
	gss_info->gss_node_idx = 1;
	input_info->input_idx = 0;

	if(!setjmp(L0_jump_buf)) {
		//printf("entered base loop initialy\n");
		//print_gss_info(rule_info->rules, gss_info);
		uint8_t first_check = 0;
		for(int i = 0; i < rule_info->rules[rule_info->rule - 65].first_size; i++) {
			if(input_info->input[input_info->input_idx] == rule_info->rules[rule_info->rule - 65].first[i]) first_check = 1;
			if('_' == rule_info->rules[rule_info->rule - 65].first[i] && input_info->input[input_info->input_idx] == '\0') first_check = 1;
		}
		if(first_check) init_rule(rule_info, input_info, gss_info, set_info);
		else return 0;
	}

	//printf("\nfell back to base loop\n");
	//print_set_info(rule_info->rules, set_info);
	
	//if(set_info->r_size == 0) {
	//	printf(
	//			"Parser is done checking success by looking for: (S, (.*), %d, *, *)\n",
	//			input_info->input_size
	//			);
	//}
	if(check_success(set_info->U_set, set_info->u_size, '0', input_info->input_size)) return 1;
	else if(set_info->r_size != 0) {
		struct descriptors curr_descriptor = set_info->R_set[--set_info->r_size];
		input_info->input_idx = curr_descriptor.input_idx;
		gss_info->gss_node_idx = curr_descriptor.gss_node_idx;
		rule_info->rule = curr_descriptor.rule;
		rule_info->start_idx = curr_descriptor.block_idx;
		rule_info->end_idx = curr_descriptor.block_end_idx;

		//printf("popping from R_set:\n");
		//print_input_info(input_info);
		//print_gss_info(rule_info->rules, gss_info);


		switch (curr_descriptor.label_type) {
			case PARTIAL_PRODUCTION:
				continue_production(
						rule_info,
						input_info,
						gss_info,
						set_info
						);
				break;
			case FULL_PRODUCTION:
				start_new_production(
						rule_info,
						input_info,
						gss_info,
						set_info
						);
				break;
			case RULE:
				init_rule(
						rule_info,
						input_info,
						gss_info,
						set_info
						);
				break;
			case BASELOOP:
				longjmp(L0_jump_buf, 1);
			default:
				printf("that shouldn't happen\n");
				exit(-1);
		}
		printf("that shouldn't happen\n");
		return -1;
	} else return 0;
}

