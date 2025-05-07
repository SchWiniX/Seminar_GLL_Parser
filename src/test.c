#undef DEBUG

#include <string.h>
#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <wchar.h>

#include "descriptor_set_functions.h"
#include "grammer_handler.h"
#include "gss.h"
#include "gll_parser.h"
#include "info_struct.h"

void print_help() {
	printf("Please provide vaild arguments:\n\n\tgll_parser_test folder [repetitions]\n\n");
	printf(" - 'folder' contains files containing grammers and input of format:\n");
	printf("X -> X_1 | X_2 | ...\n");
	printf("Y -> Y_1 | Y_2 | ...\n");
	printf("...\n");
	printf("#\n");
	printf("[test input] [0/1]\n");
	printf("...\n");
	printf(" - 'repetition' is a natural number denoting the amount of times the same input is parsed (time is then averaged)\n");
	exit(1);
}

int main(int argc, char* argv[]) {
	int repetitions = 1;
	switch (argc) {
		case 2:
			printf("defaulting to 1 repetition\n");
			break;
		case 3:
			if(!(repetitions = atoi(argv[2]))) {
				printf("Faulty input for repetition\n");
				print_help();
			}
			if(repetitions <= 0) {
				printf("Faulty input for repetition\n");
				exit(1);
			}
			break;
		default: print_help();
	}
	struct dirent* de;
	DIR* dr = opendir(argv[1]);
	assert(dr);
	printf("Grammar:input_size:Result:Should:Clock ticks:CPU Time:R Set size:U Set size:P Set size:gss_nodes size:gss_edges size:status\n");

	while ((de = readdir(dr)) != NULL) {
		if(!strncmp(de->d_name, ".", 3)) continue;
		if(!strncmp(de->d_name, "..", 3)) continue;


		uint32_t name_len = strlen(de->d_name);
		char namebuff[strlen(argv[1]) + name_len + 10];
		namebuff[0] = '\0';
		strcat(namebuff, argv[1]);
		strcat(namebuff, de->d_name);

		clock_t rule_init_ticks = clock();
		FILE* grammer_file = fopen(namebuff, "r");
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

		char next_char;
		while((next_char = fgetc(grammer_file)) != EOF) {
			uint32_t input_alloc_size = 128;
			uint32_t input_size = 0;
			char* input = malloc(input_alloc_size);
			assert(input);
			uint8_t should;
			if(next_char != '\n')
				input[input_size++] = next_char;
			while((next_char = fgetc(grammer_file)) != ' ') {
				if(next_char == EOF) break;
				if(input_size >= input_alloc_size) {
					input_alloc_size *= 2;
					input = realloc(input, input_alloc_size);
					assert(input);
				}
				input[input_size++] = next_char;
			}
			if(next_char == EOF) break;
			if(input_size >= input_alloc_size) {
					input_alloc_size *= 2;
					input = realloc(input, input_alloc_size);
					assert(input);
			}
			input[input_size] = '\0';
			switch (fgetc(grammer_file)) {
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

				free_desc_set(set_info.R_set);
				set_info.R_set = NULL;
				free_desc_set(set_info.U_set);
				set_info.U_set = NULL;
				free_p_set_entry_set(set_info.P_set);
				set_info.P_set = NULL;
				free_gss(gss_info.gss_nodes, gss_info.gss_edges);
				gss_info.gss_nodes = NULL;
				gss_info.gss_edges = NULL;
			}
			free(input);

			char* success;
			if(final_res == should) success = "\x1b[32mpassed\x1b[0m\n";
			else success = "\x1b[31mfailed\x1b[0m\n";
			printf(
					"%s:%d:%d:%d:%ld:%.3lf ms:%.2lf kB:%.2lf kB:%.2lf kB:%.2lf kB:%.2lf kB:%s",
					de->d_name,
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
		free_rules(rules);
	}
}
