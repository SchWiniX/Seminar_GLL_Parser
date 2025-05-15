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
#include "info_struct.h"

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
		U_set[u_higher_idx].alternative_start_idx == U_set[u_higher_idx].alternative_end_idx;
	int its_lower = U_set[u_lower_idx].rule == rule &&
		U_set[u_lower_idx].input_idx == input_idx &&
		U_set[u_lower_idx].alternative_start_idx == U_set[u_lower_idx].alternative_end_idx;
	return its_higher || its_lower;
}

#pragma GCC diagnostic ignored "-Winfinite-recursion"
void continue_alternative(
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
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

#ifdef DEBUG
	printf("entered continue_alternative\n");
#endif

	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
	uint16_t alternative_start_idx = rule_info->alternative_start_idx;
	uint16_t alternative_end_idx = rule_info->alternative_end_idx;
	if(alternative_start_idx == alternative_end_idx) {
		pop(input_info, gss_info, set_info);
		longjmp(L0_jump_buf, 1); //goback to base_loop
	}
	if(!is_non_terminal(this_rule.alternatives[alternative_start_idx])) {
		if(this_rule.alternatives[alternative_start_idx] == input_info->input[input_info->input_idx]) {
			input_info->input_idx += 1;
			rule_info->alternative_start_idx += 1;
			add_descriptor(rule_info, set_info, input_info->input_idx, gss_info->gss_node_idx, PARTIAL_ALTERNATIVE);
			longjmp(L0_jump_buf, 1);
			//continue_alternative(rule_info, input_info, gss_info, set_info);
		} else longjmp(L0_jump_buf, 1); //goback to base_loop
	} else {
		if(first_follow_test(rule_info, input_info->input[input_info->input_idx])) {
			rule_info->alternative_start_idx += 1;
			gss_info->gss_node_idx = create(
					rule_info,
					input_info,
					gss_info,
					set_info,
					PARTIAL_ALTERNATIVE
					);
			rule_info->alternative_start_idx -= 1;
			rule_info->rule = this_rule.alternatives[alternative_start_idx];
			init_rule(rule_info, input_info, gss_info, set_info);
		} else longjmp(L0_jump_buf, 1); //goback to base_loop
	}
}


void start_new_alternative(
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
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

#ifdef DEBUG
	printf("entered start_new_alternative\n");
#endif

	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
	uint16_t alternative_start_idx = rule_info->alternative_start_idx;
	uint16_t alternative_end_idx = rule_info->alternative_end_idx;
	if(alternative_start_idx + 1 == alternative_end_idx && this_rule.alternatives[alternative_start_idx] == '_') {
		pop(input_info, gss_info, set_info);
		longjmp(L0_jump_buf, 1); //goback to base_loop
	} else if(!is_non_terminal(this_rule.alternatives[alternative_start_idx])) {
		input_info->input_idx += 1;
		rule_info->alternative_start_idx += 1;
		add_descriptor(rule_info, set_info, input_info->input_idx, gss_info->gss_node_idx, PARTIAL_ALTERNATIVE);
		longjmp(L0_jump_buf, 1); //goback to base_loop
		//continue_alternative(rule_info, input_info, gss_info, set_info);
	} else {
		rule_info->alternative_start_idx += 1;
		gss_info->gss_node_idx = create(
				rule_info,
				input_info,
				gss_info,
				set_info,
				PARTIAL_ALTERNATIVE
				);
		rule_info->alternative_start_idx -= 1;
		rule_info->rule = this_rule.alternatives[alternative_start_idx];
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
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

#ifdef DEBUG
	printf("entered init_rule for %c\n", rule_info->rule);
#endif

	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
	for(int i = 0; i < this_rule.number_of_alternatives; i++) {
		rule_info->alternative_start_idx = this_rule.alternative_sizes[i];
		rule_info->alternative_end_idx = this_rule.alternative_sizes[i + 1];
		if(first_follow_test(rule_info, input_info->input[input_info->input_idx])) {
			add_descriptor(rule_info, set_info, input_info->input_idx, gss_info->gss_node_idx, WHOLE_ALTERNATIVE);
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
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	uint64_t first_node_idx = GET_GSS_IDX(91, 1, input_info->input_size);
	uint64_t second_node_idx = GET_GSS_IDX(91, 0, input_info->input_size);

	gss_info->gss[first_node_idx].edge_arr = malloc(1 * sizeof(gss_edge));
	gss_info->gss[first_node_idx].size = 0;
	gss_info->gss[first_node_idx].alloc_size = 1;
	gss_info->gss[second_node_idx].edge_arr = malloc(1 * sizeof(gss_edge));
	gss_info->gss[second_node_idx].size = 1;
	gss_info->gss[second_node_idx].alloc_size = 1;

	gss_info->gss[second_node_idx].edge_arr[0].target_node.rule = 92;
	gss_info->gss[second_node_idx].edge_arr[0].target_node.input_idx = 0;
	gss_info->gss[second_node_idx].edge_arr[0].rule = '0';
	gss_info->gss[second_node_idx].edge_arr[0].alternative_start_idx = 0;
	gss_info->gss[second_node_idx].edge_arr[0].alternative_end_idx = 0;
	gss_info->gss[second_node_idx].edge_arr[0].label_type = BASELOOP;

	gss_info->gss_node_idx.rule = 91;
	gss_info->gss_node_idx.input_idx = 0;
	input_info->input_idx = 0;

	if(!setjmp(L0_jump_buf)) {

#ifdef DEBUG
		printf("entered base loop initialy\n");
		print_gss_info(rule_info->rules, gss_info, input_info);
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
		rule_info->alternative_start_idx = curr_descriptor.alternative_start_idx;
		rule_info->alternative_end_idx = curr_descriptor.alternative_end_idx;

		if(curr_descriptor.input_idx > set_info->lesser_input_idx) {
			clean_lesser_from_P(set_info);
			clean_lesser_from_U(set_info);
		}

#ifdef DEBUG
		printf("popping from R_set:\n");
		print_input_info(input_info);
		print_gss_info(rule_info->rules, gss_info, input_info);
#endif


		switch (curr_descriptor.label_type) {
			case PARTIAL_ALTERNATIVE:
				continue_alternative(
						rule_info,
						input_info,
						gss_info,
						set_info
						);
				break;
			case WHOLE_ALTERNATIVE:
				start_new_alternative(
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
