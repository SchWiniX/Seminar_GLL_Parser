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
#include "debug.h"

int print_struct_info() {
	printf("-----------------------------\nSize of Descriptors: %ld bytes\n", sizeof(descriptors));
	printf("Size of gss_node: %ld bytes\n", sizeof(gss_node));
	printf("Size of gss_edge: %ld bytes\n", sizeof(gss_edge));
	printf("Size of rules: %ld bytes\n", sizeof(rule));
	return 0;
}

void print_help() {
	printf("usage: gll_parser 'grammer_file' 'input'\n\n");
	printf(" - 'grammer_file' is a path to a file containing a grammer of format:\n");
	printf("\tX -> X_1 | X_2 | ...\n");
	printf("\tY -> Y_1 | Y_2 | ...\n");
	printf("\t...\n");
	printf(" - 'input' is a string that the parser will be run on\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	clock_t ticks = clock();
	switch (argc) {
		case 3:
			break;
		default: print_help();
	}

#ifdef DEBUG
	print_struct_info();
	printf("%d arguments\n", argc);
	for(int i = 0; i < argc; i++)
		printf(" %d: %s\n", i, argv[i]);
#endif

	FILE* grammer_file = fopen(argv[1], "r");
	assert(grammer_file);

	//parse grammer input
	rule rules[26];
	uint8_t temp_vals[26];
	for(int i = 0; i < 26; i++) {
		rules[i].name = '\0';
		rules[i].first[0] = 0;
		rules[i].first[1] = 0;
		rules[i].follow[0] = 0;
		rules[i].follow[1] = 0;
		temp_vals[i] = 0;
	}
	create_grammer(rules, grammer_file);

	//find first
	for(int i = 0; i <26; i++) {
		if(rules[i].name == i + 65) {
			for(int i = 0; i < 26; i++) {
				temp_vals[i] = 0;
			}
			create_first(rules, i + 65, rules[i].first, temp_vals);
		}
	}

	//find follow
	for(int i = 0; i < 26; i++) {
		if(rules[i].name == i + 65) {
			for(int i = 0; i < 26; i++) {
				temp_vals[i] = 0;
			}		
			create_follow(rules, i + 65, rules[i].follow, temp_vals);
		}
	}
	fclose(grammer_file);
	grammer_file = NULL;

#ifdef DEBUG
	print_rules(rules);
#endif

	uint32_t gss_node_alloc_size = 1024;
	uint32_t gss_edge_alloc_size = 2048;
	uint16_t r_alloc_size = 128;
	uint16_t u_alloc_size = 512;
	uint32_t p_alloc_size = 128;

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
		.lesser_input_idx = 0,
		.p_size = 0,
		.p_lower_idx = p_alloc_size >> 1,
		.p_higher_idx = p_alloc_size >> 1,
		.p_alloc_size = p_alloc_size,
		.r_size = 0,
		.r_lower_idx = r_alloc_size >> 1,
		.r_higher_idx = r_alloc_size >> 1,
		.r_alloc_size = r_alloc_size,
		.u_size = 0,
		.u_lower_idx = u_alloc_size >> 1,
		.u_higher_idx = u_alloc_size >> 1,
		.u_alloc_size = u_alloc_size,
	};

	int res = base_loop(&rule_info, &input_info, &gss_info, &set_info);
	printf("------------------\nParsing result: %d\n------------------\n", res);

	ticks = clock() - ticks;
	printf("Time taken %ld clock ticks, %lf ms\n", ticks, ((double) ticks) * 1000/ CLOCKS_PER_SEC);

	if(free_desc_set(set_info.R_set)) {
		printf("failed to free R_set likely a memory leak\n");
	}
	set_info.R_set = NULL;
	if(free_desc_set(set_info.U_set)) {
		printf("failed to free U_set likely a memory leak\n");
	}
	set_info.U_set = NULL;
	if(free_p_set_entry_set(set_info.P_set)) {
		printf("failed to free P_set likely a memory leak\n");
	}
	set_info.P_set = NULL;
	if(free_gss(gss_info.gss_nodes, gss_info.gss_edges)) {
		printf("failed to free gss likely a memory leak\n");
	}

	gss_info.gss_nodes = NULL;
	gss_info.gss_edges = NULL;
	if(free_rules(rules)) {
		printf("failed to free rules likely a memory leak\n");
	}
	return 0;
}
