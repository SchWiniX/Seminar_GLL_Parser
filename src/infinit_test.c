#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <wchar.h>

#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include "gll_parser.h"
#include "info_struct.h"
#include "debug.h"

int main(int argc, char* argv[]) {
	if(argc < 2) {
		printf("Wrong number of arguments.\n");
		return 1;
	}


	uint32_t gen_size = argc - 2;
	uint32_t repetition_counter = 1;
	char** input_generator = argv + 2;
	uint8_t should = 1;
	printf("Input generator: ");
	for(int i = 0; i < gen_size; i++) {
		if(i % 2 == 1) printf("*(%s)", input_generator[i]);
		else printf("%s", input_generator[i]);
	}
	printf("\n");
	printf("Grammar\t\tinput_size\tResult\tShould\tClock ticks\tTime\t\tU Set\t\tP Set\t\tgss_nodes\tgss_edges\tstatus\n");
	printf("-----------------------------------------------------------------------------------------------------------------------------------------------\n");

	clock_t rule_init_ticks = clock();
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
	for(int i = 0; i < 26; i++) {
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
	
			create_follow(rules, i + 65, follow_buff, &follow_size, &follow_alloc_size, temp_vals);
			rules[i].follow = follow_buff;
			rules[i].follow_size = follow_size;
		}
	}
	rule_init_ticks = clock() - rule_init_ticks;

	while(1) {
		int input_size = 0;
		for(int i = 0; i < gen_size; i ++) {
			if(i % 2 == 1) input_size += strlen(input_generator[i]) * repetition_counter;
			else input_size += strlen(input_generator[i]);
		}
		char* input = (char*) malloc(input_size + 1);
		input[0] = '\0';
		for(int i = 0; i < gen_size; i++) {
			if(i % 2 == 1) {
				for(int j = 0; j < repetition_counter; j++) {
					strcat(input, input_generator[i]);
				}
			} else {
				strcat(input, input_generator[i]);
			}

		}
		repetition_counter += 1;
		input[input_size] = '\0';
		
		clock_t ticks = clock();

		uint32_t gss_node_alloc_size = 1024;
		uint32_t gss_edge_alloc_size = 2048;
		uint16_t r_alloc_size = 128;
		uint16_t u_alloc_size = 512;
		uint32_t p_alloc_size = 128;
	
		gss_node* gss_nodes = init_node_array(gss_node_alloc_size);
		gss_edge* gss_edges = init_edge_array(gss_edge_alloc_size);
		descriptors* R_set = init_descriptor_set(r_alloc_size);
		descriptors* U_set = init_descriptor_set(u_alloc_size);
		p_set_entry* P_set = init_p_set_entry_set(p_alloc_size);
	
		struct rule_info rule_info = { .rules = rules, .rule = 'S', . start_idx = 0, .end_idx = 0 };
		struct input_info input_info = { .input = input, .input_idx = 0, .input_size = input_size }; 
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
			.r_size = 0,
			.r_lower_idx = r_alloc_size >> 1,
			.r_higher_idx = r_alloc_size >> 1,
			.r_alloc_size = r_alloc_size,
			.u_size = 0,
			.u_lower_idx = u_alloc_size >> 1,
			.u_higher_idx = u_alloc_size >> 1,
			.u_alloc_size = u_alloc_size,
			.p_alloc_size = p_alloc_size,
			.p_size = 0,
		};

		uint8_t res = base_loop(&rule_info, &input_info, &gss_info, &set_info);
		ticks = clock() - ticks + rule_init_ticks;

		printf(
				" %s\t%10d\t%4d\t%4d\t%5ld ticks\t%.3lf ms\t%.2lf kB\t\t%.2lf kB\t\t%.2lf kB\t%.2lf kB\t",
				argv[1],
				input_size,
				res,
				should,
				ticks,
				(double) ticks * 1000 / CLOCKS_PER_SEC,
				(double) set_info.u_alloc_size * sizeof(descriptors) / 1024,
				(double) set_info.p_alloc_size * sizeof(p_alloc_size) / 1024,
				(double) gss_info.gss_node_alloc_array_size * sizeof(gss_node) / 1024,
				(double) gss_info.gss_edge_alloc_array_size * sizeof(gss_edge) / 1024
				);
	
		if(res == should) printf("\x1b[32m" "passed" "\x1b[0m\n");
		else printf("\x1b[31m" "failed" "\x1b[0m\n");
		free_desc_set(set_info.R_set);
		set_info.R_set = NULL;
		free_desc_set(set_info.U_set);
		set_info.U_set = NULL;
		free_p_set_entry_set(set_info.P_set);
		set_info.P_set = NULL;
		free_gss(gss_info.gss_nodes, gss_info.gss_edges);
		gss_info.gss_nodes = NULL;
		gss_info.gss_edges = NULL;
		free(input);
	}
	free_rules(rules);
}
