#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <wchar.h>

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
		if(1/*test*/) {
			*gss_node_idx = create(gss_nodes, gss_edges, R_set, U_set, P_set, rule, block_idx, *input_idx, *gss_node_idx);
			//goto L_A
		} else {
			//goto L_0
		}
	}
	return 0;
}


int code(
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
		) { // haha the funi function is called code

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
	*gss_node_idx = create(gss_nodes, gss_edges, R_set, U_set, P_set, rule, block_start_idx, *input_idx, *gss_node_idx);

	return 0;
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

	ticks = clock() - ticks;
	printf("Time taken %ld clock ticks, %lf ms\n", ticks, ((double) ticks) * 1000/ CLOCKS_PER_SEC);
	//TODO: implement main loop of the Parser
	return 0;
}
