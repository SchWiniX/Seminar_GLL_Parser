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

void print_help() {
	printf("Please provide vaild arguments:\n\n\tgll_parser_test grammer_file repetitions count op substr0 substr1 ...\n\n");
	printf(" - 'grammer_file' should be a path to a file containing grammers of format:\n");
	printf("X -> X_1 | X_2 | ...\n");
	printf("Y -> Y_1 | Y_2 | ...\n");
	printf("...\n");
	printf("the grammer will be done reading if it reads EOF or a terminal on the LHS\n");
	printf(" - 'repetition' is a natural number denoting the amount of times the same input is parsed (time is then averaged)\n");
	printf(" - 'count' will be the amounts of tests (-1 for non termination)\n");
	printf(" - 'op' is of form \"+N\" or \"*N\" (N being a natural umber) denoting the growth of the input per new test\n");
	printf(" - 'substr0...n' the input given to the parse is the concatination of these substring where all substr's with odd indicies are repeated\n");
	exit(1);
}

int main(int argc, char* argv[]) {
	int repetitions = 1;
	int count = -1;
	int inc_num = 1;
	char op = '\0';
	switch (argc) {
		case 1: print_help();
		case 2: print_help();
		case 3: print_help();
		case 4: print_help();
		case 5: print_help();
		default:
			if(!(repetitions = atoi(argv[2]))) {
				printf("Faulty input for repetition\n");
				print_help();
			}
			if(repetitions <= 0) {
				printf("please provide a strictly position amout of repetitions\n");
				print_help();
			}
			if(!(count = atoi(argv[3]))) {
				printf("Faulty input for count\n");
				print_help();
			}
			if(count == 0 || count < -1) {
				printf("please provide a strictly position amout of count \n");
				print_help();
			}
			if((argv[4][0] != '*' && argv[4][0] != '+') || !(inc_num = atoi(argv[4] + 1))) {
				printf("Faulty input for op\n");
				print_help();
			}
			op = argv[4][0];
			if(inc_num <= 0) {
				printf("please provide a strictly position amout of repetitions\n");
				print_help();
			}
			break;
	}


	uint32_t gen_size = argc - 5;
	uint32_t repetition_counter = 1;
	char** input_generator = argv + 5;
	uint8_t should = 1;
	printf("Input generator: ");
	for(int i = 0; i < gen_size; i++) {
		if(i % 2 == 1) printf("*(%s)", input_generator[i]);
		else printf("%s", input_generator[i]);
	}
	printf("\n");
	printf("Grammar:input_size:Result:Should:Clock ticks:CPU Time:R Set size:U Set size:P Set size:gss_nodes size:gss_edges size:status\n");

	clock_t rule_init_ticks = clock();
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
	}
	create_grammer(rules, grammer_file);
	//find first
	for(int i = 0; i < 26; i++) {
		if(rules[i].name == i + 'A') {
			for(int i = 0; i < 26; i++) {
				temp_vals[i] = 0;
			}		
			create_first(rules, i + 'A', rules[i].first, temp_vals);
		}
	}
	
	//find follow
	for(int i = 0; i < 26; i++) {
		if(rules[i].name == i + 'A') {
			for(int i = 0; i < 26; i++) {
				temp_vals[i] = 0;
			}		
			create_follow(rules, i + 'A', rules[i].follow, temp_vals);
		}
	}
	rule_init_ticks = clock() - rule_init_ticks;
	fclose(grammer_file);
	grammer_file = NULL;

	while(count) {
		if(count != -1) count -= 1;
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
		if(op == '+') {
			repetition_counter += inc_num;
		} else {
			repetition_counter *= inc_num;
		}
		input[input_size] = '\0';
		
		//repetition

		uint32_t gss_nodes_final_alloc_size;
		uint32_t gss_edge_final_alloc_size;
		uint16_t r_final_alloc_size;
		uint16_t u_final_alloc_size;
		uint16_t p_final_alloc_size;
		uint8_t final_res;
		uint64_t tick_sum = 0;
		for(int i = 0; i < repetitions; i++) {
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

			uint8_t res = base_loop(&rule_info, &input_info, &gss_info, &set_info);
			ticks = clock() - ticks + rule_init_ticks;

			tick_sum += ticks;
			gss_nodes_final_alloc_size = gss_info.gss_node_alloc_array_size;
			gss_edge_final_alloc_size = gss_info.gss_edge_alloc_array_size;
			r_final_alloc_size = set_info.r_alloc_size;
			u_final_alloc_size = set_info.u_alloc_size;
			p_final_alloc_size = set_info.p_alloc_size;
			final_res = res;
		
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
		}
		free(input);
		input = NULL;
		char* success;
		if(final_res == should) success = "\x1b[32mpassed\x1b[0m\n";
		else success = "\x1b[31mfailed\x1b[0m\n";

		printf(
				"%s:%d:%d:%d:%ld:%.3lf ms:%.2lf kB:%.2lf kB:%.2lf kB:%.2lf kB:%.2lf kB:%s",
				argv[1],
				input_size,
				final_res,
				should,
				tick_sum / repetitions,
				(double) tick_sum / repetitions * 1000 / CLOCKS_PER_SEC,
				(double) r_final_alloc_size * sizeof(descriptors) / 1024,
				(double) u_final_alloc_size * sizeof(descriptors) / 1024,
				(double) p_final_alloc_size * sizeof(p_set_entry) / 1024,
				(double) gss_nodes_final_alloc_size* sizeof(gss_node) / 1024,
				(double) gss_edge_final_alloc_size * sizeof(gss_edge) / 1024,
				success
		);
	}
	if(free_rules(rules)) {
		printf("failed to free rules likely a memory leak\n");
	}
}
