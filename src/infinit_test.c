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
	printf("usage: gll_parser_test 'grammer_file' 'repetitions' 'count' 'input_repetition_start' 'op' substr0 substr1 ...\n\n");
	printf(" - 'grammer_file' should be a path to a file containing grammers of format:\n");
	printf("\tX -> X_1 | X_2 | ...\n");
	printf("\tY -> Y_1 | Y_2 | ...\n");
	printf("\t...\n");
	printf("the grammer will be done reading if it reads EOF or a terminal on the LHS\n");
	printf(" - 'repetition' is a natural number denoting the amount of times the same input is parsed (time is then averaged)\n");
	printf(" - 'count' will be the amounts of tests (-1 for non termination)\n");
	printf(" - 'input_repetition_start' is a natural number denoting the the amount each repeating substring is already repeated at the beginning\n");
	printf(" - 'op' is of form \"+N\" or \"*N\" (N being a natural umber) denoting the growth of the input per new test\n");
	printf(" - 'substr0...n' the input given to the parse is the concatination of these substring where all substr's with odd indicies are repeated\n");
	exit(1);
}

int main(int argc, char* argv[]) {
	int repetitions = 1;
	int count = -1;
	int input_rep_start = 1;
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
				printf("please provide a strictly positive amout of repetitions\n");
				print_help();
			}
			if(!(count = atoi(argv[3]))) {
				printf("Faulty input for count\n");
				print_help();
			}
			if(count == 0 || count < -1) {
				printf("please provide a strictly positive amout of count \n");
				print_help();
			}
			if(!(input_rep_start = atoi(argv[4]))) {
				printf("Faulty input for input repetition start\n");
				print_help();
			}
			if(input_rep_start <= 0) {
				printf("please provide a strictly positive number for input start repetitions\n");
				print_help();
			}
			if((argv[5][0] != '*' && argv[5][0] != '+') || !(inc_num = atoi(argv[5] + 1))) {
				printf("Faulty input for op\n");
				print_help();
			}
			op = argv[5][0];
			if(inc_num <= 0) {
				printf("please provide a strictly position amout of repetitions\n");
				print_help();
			}
			break;
	}


	uint32_t gen_size = argc - 6;
	uint32_t repetition_counter = input_rep_start;
	char** input_generator = argv + 6;
	uint8_t should = 1;
	printf("Input generator: ");
	for(int i = 0; i < gen_size; i++) {
		if(i % 2 == 1) printf("*(%s)", input_generator[i]);
		else printf("%s", input_generator[i]);
	}
	printf("\n");
	printf("Grammar:input_size:Result:Should:CPU Time:Speed:R alloc:U total size:P total size:gss_nodes:gss_edges:gss_alloc:avg edges:avg U:avg:P:gss_ratio:status\n");

	clock_t rule_init_ticks = clock();
	FILE* grammer_file = fopen(argv[1], "r");
	assert(grammer_file);

	//parse grammer input
	rule rules[28];
	uint8_t temp_vals[26];
	for(int i = 0; i < 26; i++) {
		rules[i].name = '\0';
		rules[i].first[0] = 0;
		rules[i].first[1] = 0;
		rules[i].follow[0] = 0;
		rules[i].follow[1] = 0;
	}
	uint8_t rule_count = 0; //amount of rules in this calc
	create_grammer(rules, grammer_file, &rule_count);

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
	fclose(grammer_file);
	grammer_file = NULL;

	rule_count += 1;
	rules[26].name = 91;
	rules[26].first[0] = 0;
	rules[26].first[1] = 0;
	rules[26].follow[0] = 0;
	rules[26].follow[1] = 0;
	rules[26].count_idx = rule_count;

	rules[27].name = 92;
	rules[27].first[0] = 0;
	rules[27].first[1] = 0;
	rules[27].follow[0] = 0;
	rules[27].follow[1] = 0;
	rules[27].count_idx = rule_count + 1;

	rule_init_ticks = clock() - rule_init_ticks;

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

		uint32_t gss_final_alloc_size = -1;
		uint32_t gss_node_count = -1;
		uint32_t gss_edge_count = -1;
		uint16_t u_set_size = -1; 
		uint16_t p_set_size = -1;
		uint16_t r_final_alloc_size;
		uint8_t final_res;
		uint64_t tick_sum = 0;
		for(int i = 0; i < repetitions; i++) {
			clock_t ticks = clock();

			uint16_t r_alloc_size = 256;
		
			struct rule_info rule_info = { .rules = rules, .rule = 'S', .alternative_start_idx = 0, .alternative_end_idx = 0 };
			struct input_info input_info = { .input = input, .input_idx = 0, .input_size = input_size }; 
			struct gss_info gss_info = {
				.gss = init_gss(rule_count, input_size),
			};

			struct set_info set_info = {
				.R_set = init_descriptor_set(r_alloc_size),
				.lesser_input_idx = 0,
				.r_size = 0,
				.r_lower_idx = r_alloc_size >> 1,
				.r_higher_idx = r_alloc_size >> 1,
				.r_alloc_size = r_alloc_size,
			};

			uint8_t res = base_loop(&rule_info, &input_info, &gss_info, &set_info, rule_count);
			ticks = clock() - ticks + rule_init_ticks;

			tick_sum += ticks;
			gss_final_alloc_size = get_gss_total_alloc_size(&gss_info, rule_count, input_size);
			gss_node_count = get_gss_node_count(&gss_info, rule_count, input_size);
			gss_edge_count = get_gss_edge_count(&gss_info, rule_count, input_size);
			u_set_size = get_u_set_total_size(&gss_info, rule_count, input_size);
			p_set_size = get_p_set_total_size(&gss_info, rule_count, input_size);
			r_final_alloc_size = set_info.r_alloc_size;
			final_res = res;
		
			if(free_desc_set(set_info.R_set)) {
				printf("failed to free R_set likely a memory leak\n");
			}
			set_info.R_set = NULL;
			if(free_gss(gss_info.gss, rule_count, input_size)) {
				printf("failed to free gss likely a memory leak\n");
			}
			gss_info.gss = NULL;
		}
		free(input);
		input = NULL;
		char* success;
		if(final_res == should) success = "\x1b[32mpassed\x1b[0m";
		else success = "\x1b[31mfailed\x1b[0m";

		double time = (double) tick_sum / repetitions * 1000 / CLOCKS_PER_SEC;
		printf(
				"%s:%d:%d:%d:%.3lf ms:%.3lf c/ms:%.2lf kB:%d:%d:%d:%d:%.2lf kB:%.2lf:%.2lf:%.1lf:%.2lf:%s\n",
				argv[1],
				input_size,
				final_res,
				should,
				time,
				input_size / time,
				(double) r_final_alloc_size * sizeof(descriptors) / 1024,
				u_set_size,
				p_set_size,
				gss_node_count,
				gss_edge_count,
				(double) gss_final_alloc_size / 1024,
				(double) gss_edge_count / gss_node_count,
				(double) u_set_size / gss_node_count,
				(double) p_set_size / gss_node_count,
				(double) gss_node_count / (GET_GSS_SIZE(rule_count, input_size)),
				success
		);
	}
	if(free_rules(rules)) {
		printf("failed to free rules likely a memory leak\n");
	}
}
