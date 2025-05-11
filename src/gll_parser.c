#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>

#include "descriptor_set_functions.h"
#include "gll_parser.h"
#include "grammer_handler.h"
#include "gss.h"
#include "debug.h"

jmp_buf L0_jump_buf;

#ifdef DEBUG
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
#endif

int check_success(descriptors U_set[], uint16_t u_lower_idx, uint16_t u_higher_idx, uint16_t u_alloc_size, uint16_t rule, uint32_t input_idx) {
	assert(U_set);
	if(u_higher_idx == 0) {
		u_higher_idx = u_alloc_size - 1;
	} else {
		u_higher_idx -= 1;
	}
	int its_higher = U_set[u_higher_idx].rule == rule &&
		U_set[u_higher_idx].input_idx == input_idx &&
		U_set[u_higher_idx].block_idx == U_set[u_higher_idx].block_end_idx;
	int its_lower = U_set[u_lower_idx].rule == rule &&
		U_set[u_lower_idx].input_idx == input_idx &&
		U_set[u_lower_idx].block_idx == U_set[u_lower_idx].block_end_idx;
	return its_higher || its_lower;
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

#ifdef DEBUG
	printf("entered continue_production\n");
#endif

	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
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
			add_descriptor(rule_info, set_info, input_info->input_idx, gss_info->gss_node_idx, PARTIAL_PRODUCTION);
			longjmp(L0_jump_buf, 1);
			//continue_production(rule_info, input_info, gss_info, set_info);
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

#ifdef DEBUG
	printf("entered start_new_production\n");
#endif

	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
	uint16_t start_idx = rule_info->start_idx;
	uint16_t end_idx = rule_info->end_idx;
	if(start_idx + 1 == end_idx && this_rule.blocks[start_idx] == '_') {
		pop(input_info, gss_info, set_info);

		longjmp(L0_jump_buf, 1); //goback to base_loop
	} else if(!is_non_terminal(this_rule.blocks[start_idx])) {
		input_info->input_idx += 1;
		rule_info->start_idx += 1;
		add_descriptor(rule_info, set_info, input_info->input_idx, gss_info->gss_node_idx, PARTIAL_PRODUCTION);
		longjmp(L0_jump_buf, 1); //goback to base_loop
		//continue_production(rule_info, input_info, gss_info, set_info);
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

#ifdef DEBUG
	printf("entered init_rule for %c\n", rule_info->rule);
#endif

	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
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
	gss_info->gss_nodes[0].input_idx = 0;

	gss_info->gss_nodes[1].rule = '0';
	gss_info->gss_nodes[1].input_idx = 0;

	gss_info->gss_edges[0].src_node = 1;
	gss_info->gss_edges[0].target_node = 0;
	gss_info->gss_edges[0].rule = 'S';
	gss_info->gss_edges[0].block_idx = 0;
	gss_info->gss_edges[0].block_end_idx = 0;
	gss_info->gss_edges[0].label_type = BASELOOP;

	gss_info->gss_node_array_size = 2;
	gss_info->gss_edge_array_size = 1;
	
	gss_info->gss_node_idx = 1;
	input_info->input_idx = 0;

	if(!setjmp(L0_jump_buf)) {

#ifdef DEBUG
		printf("entered base loop initialy\n");
		print_gss_info(rule_info->rules, gss_info);
#endif

		char first_char = input_info->input[0];
		uint8_t first_check = is_in_first_follow(rule_info->rules[rule_info->rule - 'A'].first, first_char);
		if(is_in_first_follow(rule_info->rules[rule_info->rule - 'A'].first, '_')) {
			first_check = first_check || first_char == '\0';
		}
		if(first_check) init_rule(rule_info, input_info, gss_info, set_info);
		else return 0;
	}

#ifdef DEBUG
	printf("\nfell back to base loop\n");
	print_set_info(rule_info->rules, set_info);
#endif	

	if(check_success(
				set_info->U_set,
				set_info->u_lower_idx,
				set_info->u_higher_idx,
				set_info->u_alloc_size,
				'0',
				input_info->input_size)) return 1;
	else if(set_info->r_size != 0) {
		struct descriptors curr_descriptor = set_info->R_set[set_info->r_lower_idx];
		set_info->r_lower_idx = (set_info->r_lower_idx + 1) % set_info->r_alloc_size;
		set_info->r_size -= 1;
		input_info->input_idx = curr_descriptor.input_idx;
		gss_info->gss_node_idx = curr_descriptor.gss_node_idx;
		rule_info->rule = curr_descriptor.rule;
		rule_info->start_idx = curr_descriptor.block_idx;
		rule_info->end_idx = curr_descriptor.block_end_idx;

		if(curr_descriptor.input_idx > set_info->lesser_input_idx) {
			clean_lesser_from_P(set_info);
			clean_lesser_from_U(set_info);
		}

#ifdef DEBUG
		printf("popping from R_set:\n");
		print_input_info(input_info);
		print_gss_info(rule_info->rules, gss_info);
#endif


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
				printf("welp that shouldn't happen\n");
				exit(-1);
		}
		printf("welp that shouldn't happen either\n");
		exit(-1);
	} else return 0;
}
