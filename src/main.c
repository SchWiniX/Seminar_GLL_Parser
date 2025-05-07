#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include "gll_parser.h"
#include "info_struct.h"

int print_struct_info() {
	printf("-----------------------------\nSize of Descriptors: %ld bytes\n", sizeof(descriptors));
	printf("Size of gss_node: %ld bytes\n", sizeof(gss_node));
	printf("Size of gss_edge: %ld bytes\n", sizeof(gss_edge));
	return 0;
}

int main(int argc, char *argv[]) {
	clock_t ticks = clock();
	//print_struct_info();
	//printf("%d arguments\n", argc);
	//for(int i = 0; i < argc; i++)
	//	printf(" %d: %s\n", i, argv[i]);
	
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

	//find first
	for(int i = 0; i <26; i++) {
		if(rules[i].name == i + 'A')
			create_first(rules, i + 'A', temp_vals);
	}

	//find follow
	for(int i = 0; i < 26; i++) {
		if(rules[i].name == i + 'A') {
			char* follow_buff = (char*) malloc(32);
			assert(follow_buff);

			uint16_t follow_size = 0;
			uint16_t follow_alloc_size = 32;
			for(int i = 0; i < 26; i++) {
				temp_vals[i] = 0;
			}		

			create_follow(rules, i + 'A', follow_buff, &follow_size, &follow_alloc_size, temp_vals);
			rules[i].follow = follow_buff;
			rules[i].follow_size = follow_size;
		}
	}


	uint16_t gss_node_alloc_size = 256;
	uint16_t gss_edge_alloc_size = 256;
	uint16_t r_alloc_size = 128;
	uint16_t u_alloc_size = 512;
	uint16_t p_alloc_size = 128;

	uint32_t input_size = strlen(argv[2]); //this feels hella unsafe... anyway moving on
	gss_node* gss_nodes = init_node_array(gss_node_alloc_size);
	gss_edge* gss_edges = init_edge_array(gss_edge_alloc_size);
	descriptors* R_set = init_descriptor_set(r_alloc_size);
	descriptors* U_set = init_descriptor_set(u_alloc_size);
	p_set_entry* P_set = init_p_set_entry_set(p_alloc_size);

	struct rule_info rule_info = { .rules = rules, .rule = 'S', . start_idx = 0, .end_idx = 0 };
	struct input_info input_info = { .input = argv[2], .input_idx = 0, .input_size = input_size }; 
	struct gss_info gss_info = {
		.gss_nodes = gss_nodes,
		.gss_edges = gss_edges,
		.gss_node_idx = 0,
		.gss_node_alloc_array_size = gss_node_alloc_size,
		.gss_edge_alloc_array_size = gss_edge_alloc_size,
		.gss_node_array_size = 0,
		.gss_edge_array_size = 0,
	};
	struct set_info set_info = {
		.R_set = R_set,
		.U_set = U_set,
		.P_set = P_set,
		.r_alloc_size = r_alloc_size,
		.u_alloc_size = u_alloc_size,
		.p_alloc_size = p_alloc_size,
		.r_size = 0,
		.u_size = 0,
		.p_size = 0,
	};

	//printf("-----------------------------\nResult:\n-----------------------------\n");
	printf("Parser returned: %d\n", base_loop(&rule_info, &input_info, &gss_info, &set_info));

	ticks = clock() - ticks;
	printf("Time taken %ld clock ticks, %lf ms\n", ticks, ((double) ticks) * 1000/ CLOCKS_PER_SEC);

	free_desc_set(set_info.R_set);
	set_info.R_set = NULL;
	free_desc_set(set_info.U_set);
	set_info.U_set = NULL;
	free_p_set_entry_set(set_info.P_set);
	set_info.P_set = NULL;
	free_gss(gss_info.gss_nodes, gss_info.gss_edges);
	gss_info.gss_nodes = NULL;
	gss_info.gss_edges = NULL;
	free_rules(rules);
	return 0;
}
