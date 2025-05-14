#include <errno.h>
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
	printf("usage: gll_parser_test 'folder' | 'grammer_file' [repetitions]\n\n");
	printf(" - 'folder' contains files containing grammers and input of format:\n");
	printf("\tX -> X_1 | X_2 | ...\n");
	printf("\tY -> Y_1 | Y_2 | ...\n");
	printf("\t...\n");
	printf("\t#\n");
	printf("\t[test input] [0/1]\n");
	printf("\t...\n");
	printf(" - 'repetition' is a natural number denoting the amount of times the same input is parsed (time is then averaged)\n");
	exit(1);
}


int do_inputs(char* file, uint32_t repetitions) {
	clock_t rule_init_ticks = clock();
	FILE* grammer_file = fopen(file, "r");
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
			if(next_char == EOF) {
				free(input);
				break;
			}
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

		uint32_t gss_final_alloc_size = -1;
		uint32_t gss_node_count = -1;
		uint32_t gss_edge_count = -1;
		uint16_t r_final_alloc_size = -1;
		uint16_t u_final_alloc_size = -1;
		uint16_t p_final_alloc_size = -1;
		uint8_t final_res = -1;
		uint64_t tick_sum = 0;
		for(int i = 0; i < repetitions; i++) {
			clock_t ticks = clock();

			uint16_t r_alloc_size = 128;
			uint16_t u_alloc_size = 512;
			uint32_t p_alloc_size = 128;
	
			gss_node* gss = init_gss(26, input_size);
			descriptors* R_set = init_descriptor_set(r_alloc_size);
			descriptors* U_set = init_descriptor_set(u_alloc_size);
			p_set_entry* P_set = init_p_set_entry_set(p_alloc_size);
	
			struct rule_info rule_info = { .rules = rules, .rule = 'S', .alternative_start_idx = 0, .alternative_end_idx = 0 };
			struct input_info input_info = { .input = input, .input_idx = 0, .input_size = input_size }; 
			struct gss_info gss_info = {
				.gss = gss,
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
			gss_final_alloc_size = get_gss_total_alloc_size(&gss_info, 26, input_size);
			gss_node_count = get_gss_node_count(&gss_info, 26, input_size);
			gss_edge_count = get_gss_edge_count(&gss_info, 26, input_size);
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
			if(free_gss(gss_info.gss, 26, input_size)) {
				printf("failed to free gss likely a memory leak\n");
			}
			gss_info.gss = NULL;
		}
		free(input);
		input = NULL;

		char* success;
		if(final_res == should) success = "\x1b[32mpassed\x1b[0m\n";
		else success = "\x1b[31mfailed\x1b[0m\n";
		printf(
				"%s:%d:%d:%d:%ld:%.3lf ms:%.2lf kB:%.2lf kB:%.2lf kB:%d:%d:%.2lf kB:%s",
				file,
				input_size,
				final_res,
				should,
				tick_sum / repetitions,
				(double) tick_sum / repetitions * 1000 / CLOCKS_PER_SEC,
				(double) r_final_alloc_size * sizeof(descriptors) / 1024,
				(double) u_final_alloc_size * sizeof(descriptors) / 1024,
				(double) p_final_alloc_size * sizeof(p_set_entry) / 1024,
				gss_node_count,
				gss_edge_count,
				(double) gss_final_alloc_size / 1024,
				success
		);
	}
	fclose(grammer_file);
	grammer_file = NULL;
	if(free_rules(rules)) {
		free_rules(rules);
	}
	return 1;
}

int do_folder(DIR* dr, char* folder, uint32_t repetitions) {
	struct dirent* de;
	printf("Grammar:input_size:Result:Should:Clock ticks:CPU Time:R alloc:U alloc:P alloc:gss_nodes:gss_edges:gss_alloc:status\n");

	while ((de = readdir(dr)) != NULL) {
		if(!strncmp(de->d_name, ".", 3)) continue;
		if(!strncmp(de->d_name, "..", 3)) continue;
		uint32_t name_len = strlen(de->d_name);
		char namebuff[strlen(folder) + name_len + 10];
		namebuff[0] = '\0';
		strcat(namebuff, folder);
		strcat(namebuff, de->d_name);
		do_inputs(namebuff, repetitions);
	}
	return 0;
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

	DIR* dr = opendir(argv[1]);
	if(dr == NULL) { //actual error handeling... miracles do happen sometimes ig
		switch (errno) {
			case EACCES:
				printf("File/folder: permission denied.\n");
			case ENOTDIR:
				do_inputs(argv[1], repetitions);
				exit(0);
				break;
			default:
				printf("'folder' | 'grammer_file' must either be a folder or file path\n");
		}
	}
	do_folder(dr, argv[1], repetitions);

	closedir(dr);
	

}
