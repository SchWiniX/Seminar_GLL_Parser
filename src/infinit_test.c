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

struct input_gen {
	char** input_parts;
	uint32_t rep_count;
	uint32_t number_of_parts;
	uint8_t first_repeat;
};

int main(int argc, char* argv[]) {
	if(argc != 3) {
		printf("Wrong number of arguments. Please provide a path to the grammar file and an input string\n");
		return 1;
	}

	printf("Grammar\tinput_size\tResult\tShould\tClock ticks\tTime\t\tstatus\n");
	printf("------------------------------------------------------------------------------\n");

	uint32_t input_len = strlen(argv[2]); //funi unsafe shit happening here
	char* input_parts[input_len];
	struct input_gen input_gen = { .input_parts = input_parts, .rep_count = 0, .number_of_parts = 0 };
	int char_count = 0;
	for(int i = 0; i < input_len; i++) {
		if(argv[2][i] == '*') {
			int count = argv[2][i + 1] - 48 + i + 2;
			input_gen.number_of_parts += 1;
			if(i == 0) input_gen.first_repeat = 0;
			else input_gen.first_repeat = 1;
			for(; i < count; i++)
				input_gen.input_parts[input_gen.number_of_parts][char_count++] = argv[2][i];
			input_gen.number_of_parts += 1;
		}
		input_gen.input_parts[input_gen.number_of_parts][char_count++] = argv[2][i];
	}
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
	rule_init_ticks = clock() - rule_init_ticks;

	char next_char;
	while(1) {
		uint32_t input_alloc_size = 128;
		uint32_t input_size = 0;
		char* input = malloc(input_alloc_size);
		assert(input);
		if(next_char != '\n')
			input[input_size++] = next_char;
		for(int i = 0; i < input_gen.number_of_parts; i++) {
			if(input_size >= input_alloc_size) {
				input_alloc_size *= 2;
				input = realloc(input, input_alloc_size);
				assert(input);
			}
			input[input_size++] = next_char;
		}
		if(input_size >= input_alloc_size) {
				input_size *= 2;
				input = realloc(input, input_alloc_size);
				assert(input);
		}
		input[input_size] = '\0';
		switch (fgetc(grammar_file)) {
			case '0':
				should = 0;
				break;
			case '1':
				should = 1;
				break;
			default:
					printf("Faulty Grammar file\n");
					exit(1);
		}

		
		clock_t ticks = clock();
		uint16_t gss_node_alloc_size = 256;
		uint16_t gss_edge_alloc_size = 256;
		uint16_t r_alloc_size = 128;
		uint16_t u_alloc_size = 512;
		uint16_t p_alloc_size = 128;
	
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
			.r_alloc_size = r_alloc_size,
			.u_alloc_size = u_alloc_size,
			.p_alloc_size = p_alloc_size,
			.r_size = 0,
			.u_size = 0,
			.p_size = 0,
		};

		uint8_t res = base_loop(&rule_info, &input_info, &gss_info, &set_info);
		ticks = clock() - ticks + rule_init_ticks;

		printf(
				" %s\t%10d\t%4d\t%4d\t%5ld ticks\t%.3lf ms\t",
				argv[1],
				input_size,
				res,
				should,
				ticks,
				(double) ticks * 1000 / CLOCKS_PER_SEC
				);
	
		if(res == should) printf("\x1b[32m" "passed\n" "\x1b[0m");
		else printf("\x1b[31m" "failed\n" "\x1b[0m");
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
