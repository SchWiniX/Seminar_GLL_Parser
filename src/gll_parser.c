#include "descriptor_set_functions.h"
#include "gll_parser.h"
#include "grammer_handler.h"
#include "gss.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf L0_jump_buf;

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
	assert(input_info->input_idx);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(gss_info->gss_node_idx);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	struct rule this_rule = rule_info->rules[rule_info->rule - 65];
	uint16_t start_idx = rule_info->start_idx;
	uint16_t end_idx = rule_info->end_idx;
	if(start_idx == end_idx) {
		pop(
				gss_info->gss_nodes,
				gss_info->gss_edges,
				set_info->R_set,
				set_info->U_set,
				set_info->P_set,
				gss_info->gss_node_idx,
				input_info->input_idx
				);
		longjmp(L0_jump_buf, 1); //goback to base_loop
	}
	if(!is_non_terminal(this_rule.blocks[start_idx])) {
		if(this_rule.blocks[start_idx] == input_info->input[input_info->input_idx]) {
			input_info->input_idx += 1;
			rule_info->start_idx += 1;
			continue_production(
					rule_info,
					input_info,
					gss_info,
					set_info
					);
		}
		else longjmp(L0_jump_buf, 1); //goback to base_loop
	} else {
		if(first_follow_test(
					rule_info->rules,
					rule_info->rule,
					rule_info->start_idx,
					rule_info->end_idx,
					input_info->input[input_info->input_idx]
					)) {
			gss_info->gss_node_idx = create(
					gss_info->gss_nodes,
					gss_info->gss_edges,
					set_info->R_set,
					set_info->U_set,
					set_info->P_set,
					input_info->input_idx,
					rule_info->start_idx + 1,
					rule_info->end_idx,
					gss_info->gss_node_idx,
					PARTIAL_PRODUCTION,
					rule_info->rule
					);
			rule_info->rule = this_rule.blocks[start_idx];
			init_rule(rule_info, input_info, gss_info, set_info);
		} else longjmp(L0_jump_buf, 1); //goback to base_loop
	}
}


int start_new_production(
		struct rule_info* rule_info,
		struct input_info* input_info,
		struct gss_info* gss_info,
		struct set_info* set_info
		) {

	assert(rule_info);
	assert(rule_info->rules);
	assert(input_info);
	assert(input_info->input);
	assert(input_info->input_idx);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(gss_info->gss_node_idx);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	struct rule this_rule = rule_info->rules[rule_info->rule - 65];
	uint16_t start_idx = rule_info->start_idx;
	uint16_t end_idx = rule_info->end_idx;
	if(start_idx + 1 == end_idx && this_rule.blocks[start_idx]) {
		pop(
				gss_info->gss_nodes,
				gss_info->gss_edges,
				set_info->R_set,
				set_info->U_set,
				set_info->P_set,
				gss_info->gss_node_idx,
				input_info->input_idx
				);

		longjmp(L0_jump_buf, 1); //goback to base_loop
	} else if(!is_non_terminal(this_rule.blocks[start_idx])) {
		input_info->input_idx += 1;
		rule_info->start_idx += 1;
		continue_production(rule_info, input_info, gss_info, set_info);
	} else {
		gss_info->gss_node_idx = create(
				gss_info->gss_nodes,
				gss_info->gss_edges,
				set_info->R_set,
				set_info->U_set,
				set_info->P_set,
				input_info->input_idx,
				rule_info->start_idx + 1,
				rule_info->end_idx,
				gss_info->gss_node_idx,
				PARTIAL_PRODUCTION,
				rule_info->rule
				);
		rule_info->rule = this_rule.blocks[start_idx];
		init_rule(rule_info, input_info, gss_info, set_info);
	}
	return 0;
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
	assert(input_info->input_idx);
	assert(gss_info);
	assert(gss_info->gss_nodes);
	assert(gss_info->gss_edges);
	assert(gss_info->gss_node_idx);
	assert(set_info);
	assert(set_info->R_set);
	assert(set_info->U_set);
	assert(set_info->P_set);

	struct rule this_rule = rule_info->rules[rule_info->rule - 65];
	for(int i = 0; i < this_rule.number_of_blocks; i++) {
		uint16_t start_idx = this_rule.block_sizes[i];
		uint16_t end_idx = this_rule.block_sizes[i + 1];
		if(first_follow_test(
					rule_info->rules,
					rule_info->rule,
					rule_info->start_idx,
					rule_info->end_idx,
					input_info->input[input_info->input_idx])) {
			add_descriptor(
					set_info->R_set,
					set_info->U_set,
					set_info->rule,
					rule_info->start_idx, 
					rule_info->end_idx, 
					input_info->input_idx, 
					gss_info->gss_node_idx, 
					FULL_PRODUCTION
					);
		}
	}
	longjmp(L0_jump_buf, 1); //goback to base_loop
}

int base_loop(
		rule rules[],
		char* input,
		uint32_t* input_idx,
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		uint16_t* gss_node_idx,
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[]
		) {

	assert(rules);
	assert(input);
	assert(input_idx);
	assert(gss_nodes);
	assert(gss_edges);
	assert(gss_node_idx);
	assert(R_set);
	assert(U_set);
	assert(P_set);

	gss_nodes[0].rule = '0';
	gss_nodes[0].block_idx = 0;
	gss_nodes[0].block_end_idx = 0;
	gss_nodes[0].input_idx = 0;
	gss_nodes[0].label_type = INVALID;

	gss_nodes[1].rule = '0';
	gss_nodes[1].block_idx = 0;
	gss_nodes[1].block_end_idx = 0;
	gss_nodes[1].input_idx = 0;
	gss_nodes[1].label_type = BASELOOP;

	gss_edges[0].src_node = 0;
	gss_edges[0].target_node = 1;

	gss_node_array_size = 2;
	gss_edge_array_size = 1;
	
	*gss_node_idx = 1;
	*input_idx = 0;

	//TODO: everything need to be packed into a struct pointer as we do realloc calls which do change the Mem address
	//but when we longjump we will reset to the inital memory addresses which is like pretty fucking bad
	if(!setjmp(L0_jump_buf)) {
		uint8_t first_check = 0;
		for(int i = 0; i < rules['S' - 65].first_size; i++) {
			if(input[*input_idx] == rules['S' - 65].first[i]) first_check = 1;
			if('_' == rules['S' - 65].first[i] && input[*input_idx] == '$') first_check = 1;
		}
		if(first_check) {
			init_rule(rules, 'S', input, input_idx, gss_nodes, gss_edges, gss_node_idx, R_set, U_set, P_set);
		} else return 0;
	}


	if(r_size != 0) {
		struct descriptors curr_descriptor = R_set[--r_size];
		*input_idx = curr_descriptor.input_idx;
		*gss_node_idx = curr_descriptor.gss_node_idx;
		switch (curr_descriptor.label_type) {
			case PARTIAL_PRODUCTION:
				continue_production(
						rules,
						curr_descriptor.rule,
						curr_descriptor.block_idx,
						curr_descriptor.block_end_idx,
						input,
						input_idx,
						gss_nodes,
						gss_edges,
						gss_node_idx,
						R_set,
						U_set,
						P_set
						);
				break;
			case FULL_PRODUCTION:
				start_new_production(rules,
						curr_descriptor.rule,
						curr_descriptor.block_idx,
						curr_descriptor.block_end_idx,
						input,
						input_idx,
						gss_nodes,
						gss_edges,
						gss_node_idx,
						R_set,
						U_set,
						P_set
						);
				break;
			case RULE:
				init_rule(
						rules,
						curr_descriptor.rule,
						input,
						input_idx,
						gss_nodes,
						gss_edges,
						gss_node_idx,
						R_set,
						U_set,
						P_set
						);
				break;
			case BASELOOP:
				longjmp(L0_jump_buf, 1);
			default:
				printf("that shouldn't happen\n");
				exit(1);
		}
		printf("that shouldn't happen\n");
		return -1;
	} else if(in_set(U_set, '0', (uint16_t) 0, (uint16_t) 0, 999, 0, BASELOOP)) {
		return 1;
	} else {
		return 0;
	}
}

