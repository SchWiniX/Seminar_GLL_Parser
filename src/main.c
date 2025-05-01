#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include <sys/types.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

int exec_rule(
		rule rules[],
		char rule,
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		gss_node gss_nodes[],
		uint16_t* gss_node_idx,
		gss_edge gss_edges[],
		char* input,
		uint32_t* input_idx
		);

int print_struct_info() {
	printf("-----------------------------\nSize of Descriptors: %ld bytes\n", sizeof(descriptors));
	printf("Size of gss_node: %ld bytes\n", sizeof(gss_node));
	printf("Size of gss_edge: %ld bytes\n", sizeof(gss_edge));
	return 0;
}

int print_rules(rule rules[]){
	for(int i = 0; i < 26; i++) {
		if(rules[i].name != (char) (i + 65)) continue;
		printf("-----------------------------\nrule: %c\n", rules[i].name);
		printf("number of blocks: %d:\n", rules[i].number_of_blocks);
		printf("blocks_sizes: ");
		printf("%d", rules[i].block_sizes[0]);
		for(int j = 1; j <= rules[i].number_of_blocks; j++) {
			printf(", %d", rules[i].block_sizes[j]);
		}
		printf("\nblocks: ");
		for(int j = 1; j <= rules[i].number_of_blocks; j++) {
			for(int k = rules[i].block_sizes[j - 1]; k < rules[i].block_sizes[j]; k++) {
				printf("%c", rules[i].blocks[k]);
			}
			if (j != rules[i].number_of_blocks) printf(", ");
		}
		printf("\n");
	}
	return 0;
}

int print_first_and_follow(rule rules[]) {
	for(int i = 0; i < 26; i++) {
		if(rules[i].name != (char) (i + 65)) continue;
		printf("-----------------------------\nfirst(%c)[%d]: { ", rules[i].name, rules[i].first_size);
		for(int j = 0; j < rules[i].first_size; j++) {
			printf("%c", rules[i].first[j]);
			if (j != rules[i].first_size - 1) printf(", ");
		}
		printf(" }\n");

		printf("follow(%c)[%d]: { ", rules[i].name, rules[i].follow_size);
		for(int j = 0; j < rules[i].follow_size; j++) {
			printf("%c", rules[i].follow[j]);
			if (j != rules[i].follow_size - 1) printf(", ");
		}
		printf(" }\n");

	}
	return 0;

}

int parse_single_char(
		rule rules[],
		char rule,
		uint16_t block_idx,
		uint16_t block_end_idx,
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		gss_node gss_nodes[],
		uint16_t* gss_node_idx,
		gss_edge gss_edges[],
		char* input,
		uint32_t* input_idx
) {

	assert(rules);
	assert(R_set);
	assert(U_set);
	assert(P_set);
	assert(gss_nodes);
	assert(gss_edges);
	assert(input);
	assert(input_idx);

	struct rule this_rule = rules[rule - 65];
	if(!is_non_terminal(this_rule.blocks[block_idx])) {
		if(input[*input_idx] == this_rule.blocks[block_idx]) {
			*input_idx += 1;
		} else {
			//goto L_0
		}
	} else {
		if(first_follow_test(rules, rule, block_idx, block_end_idx, input[*input_idx])) {
			*gss_node_idx = create(
					gss_nodes,
					gss_edges,
					R_set,
					U_set,
					P_set,
					rule,
					block_idx,
					block_end_idx,
					*input_idx,
					*gss_node_idx,
					EMPTY
					);
			exec_rule(
					rules,
					rule,
					R_set,
					U_set,
					P_set,
					gss_nodes,
					gss_node_idx,
					gss_edges,
					input,
					input_idx
					); //equiv goto L_rule
		} else {
			//goto L_0
		}
	}
	return 0;
}


int exec_production(
		rule rules[],
		char rule,
		uint16_t block_start_idx,
		uint16_t block_end_idx,
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		gss_node gss_nodes[],
		uint16_t* gss_node_idx,
		gss_edge gss_edges[],
		char* input,
		uint32_t* input_idx
		) {

	assert(rules);
	assert(R_set);
	assert(U_set);
	assert(P_set);
	assert(gss_nodes);
	assert(gss_edges);
	assert(input);
	assert(input_idx);
	assert(block_start_idx < block_end_idx);
	
	struct rule this_rule = rules[rule - 65];
	if(block_start_idx + 1 == block_end_idx && this_rule.blocks[block_start_idx] == '_') {
		pop(gss_nodes, gss_edges, R_set, U_set, P_set, *gss_node_idx, *input_idx);
		//goto L_0
	}
	if (!is_non_terminal(rule)) {
		input_idx += 1;
		for(int i = block_start_idx; i < block_end_idx; i++) {
			parse_single_char(
					rules,
					rule,
					i,
					block_end_idx,
					R_set,
					U_set,
					P_set,
					gss_nodes,
					gss_node_idx,
					gss_edges,
					input,
					input_idx
					);
		}
		pop(gss_nodes, gss_edges, R_set, U_set, P_set, *gss_node_idx, *input_idx);
		//goto L_0
	}

	*gss_node_idx = create(
			gss_nodes,
			gss_edges,
			R_set,
			U_set,
			P_set,
			rule,
			block_start_idx,
			block_end_idx,
			*input_idx,
			*gss_node_idx,
			DO_SINGLE_PARSES
			);

	exec_rule(
			rules,
			rule,
			R_set,
			U_set,
			P_set,
			gss_nodes,
			gss_node_idx,
			gss_edges,
			input,
			input_idx
			); //equiv goto L_rule

	return 0;
}

int exec_rule(
		rule rules[],
		char rule,
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		gss_node gss_nodes[],
		uint16_t* gss_node_idx,
		gss_edge gss_edges[],
		char* input,
		uint32_t* input_idx
		) {

	assert(rules);
	assert(R_set);
	assert(U_set);
	assert(P_set);
	assert(gss_nodes);
	assert(gss_edges);
	assert(input);
	assert(input_idx);

	struct rule this_rule = rules[rule - 65];
	for(int i = 1; i <= this_rule.number_of_blocks; i++) {
		uint16_t start_idx = this_rule.block_sizes[i];
		uint16_t end_idx = this_rule.block_sizes[i - 1];
		if(first_follow_test(rules, rule, start_idx, end_idx, input[*input_idx])) 
			add_descriptor(
					R_set,
					U_set,
					rule,
					start_idx,
					end_idx,
					*input_idx,
					*gss_node_idx,
					DO_PRODUCTION
					);
	}
	//goto L0

}

int exec_label_descriptor(
		rule rules[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		descriptors curr_descriptor,
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		char* input
		) {
	
	assert(rules);
	assert(R_set);
	assert(U_set);
	assert(P_set);
	assert(gss_nodes);
	assert(gss_edges);
	assert(input);
	assert(curr_descriptor.block_idx < curr_descriptor.block_end_idx);


	uint32_t* input_idx;
	*input_idx = curr_descriptor.input_idx;
	uint16_t* gss_node_idx;
	*gss_node_idx = curr_descriptor.gss_node_idx;
	switch (curr_descriptor.label_type) {
		case EMPTY:
			break;
		case DO_SINGLE_PARSES:
			for(int i = curr_descriptor.block_idx; i < curr_descriptor.block_end_idx; i++) {
				parse_single_char(
						rules,
						curr_descriptor.rule,
						i,
						curr_descriptor.block_end_idx,
						R_set,
						U_set,
						P_set,
						gss_nodes,
						gss_node_idx,
						gss_edges,
						input,
						input_idx
						);
			}
			break;
		case DO_PRODUCTION:
			exec_production(
					rules,
					curr_descriptor.rule,
					curr_descriptor.block_idx,
					curr_descriptor.block_end_idx,
					R_set,
					U_set,
					P_set,
					gss_nodes,
					gss_node_idx,
					gss_edges,
					input,
					input_idx
					);
			break;
	}
	return 0;
}

int gll_l0(
		rule rules[],
		descriptors R_set[],
		descriptors U_set[],
		p_set_entry P_set[],
		gss_node gss_nodes[],
		gss_edge gss_edges[],
		char* input
		) {

	if(r_size != 0) {
		struct descriptors curr_descriptor = R_set[r_size];
		r_size -= 1;
		exec_label_descriptor(
			rules,
			R_set,
			U_set,
			P_set,
			curr_descriptor,
			gss_nodes,
			gss_edges,
			input
			);
	} else if (1/* test in U */) return 0;
	else return 1;

}

int main(int argc, char *argv[]) {
	clock_t ticks = clock();
	print_struct_info();
	printf("%d arguments\n", argc);
	for(int i = 0; i < argc; i++)
		printf(" %d: %s\n", i, argv[i]);
	
	if(argc != 3) {
		printf("Wrong number of arguments. Please provide a path to the grammar file and an input string\n");
		return 1;
	}

	FILE* grammar_file = fopen(argv[1], "r");
	assert(grammar_file);

	//parse grammer input
	rule rules[26];
	uint8_t temp_vals[26];
	for(int i = 0; i < 26; i++) {
		rules[i].name = '\0';
		rules[i].first = NULL;
		temp_vals[i] = 0;
	}
	create_grammar(rules, grammar_file);
	print_rules(rules);

	//find first
	for(int i = 0; i <26; i++) {
		if(rules[i].name == i + 65)
			create_first(rules, i + 65, temp_vals);
	}

	//find follow
	for(int i = 0; i < 26; i++) {
		if(rules[i].name == i + 65) {
			char* follow_buff = (char*) malloc(32);
			assert(follow_buff);

			uint16_t follow_size = 0;
			uint16_t follow_alloc_size = 32;
			for(int i = 0; i < 26; i++) {
				temp_vals[i] = 0;
			}		

			//printf("****** started follow of rule %c ******\n", i + 65);
			create_follow(rules, i + 65, follow_buff, &follow_size, &follow_alloc_size, temp_vals);
			rules[i].follow = follow_buff;
			rules[i].follow_size = follow_size;
			//printf("****** ended follow of rule %c ******\n", i + 65);
		}
	}
	print_first_and_follow(rules);

	//TODO: Actually fucking parse
	uint32_t input_size = 0;
	gss_node* gss_nodes = init_node_array();
	gss_edge* gss_edges = init_edge_array();
	descriptors* R_set = init_set(r_total_size);
	descriptors* U_set = init_set(u_total_size);
	p_set_entry* P_set = init_p_set_entry_set(p_total_size);

	gss_nodes[0]

	ticks = clock() - ticks;
	printf("Time taken %ld clock ticks, %lf ms\n", ticks, ((double) ticks) * 1000/ CLOCKS_PER_SEC);
	//TODO: implement main loop of the Parser
	return 0;
}
