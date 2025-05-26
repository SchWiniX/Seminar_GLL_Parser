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
	printf("usage: gll_parser 'grammer_file' 'flag' 'input'\n\n");
	printf(" - 'grammer_file' is a path to a file containing a grammer of format:\n");
	printf("\tX -> X_1 | X_2 | ...\n");
	printf("\tY -> Y_1 | Y_2 | ...\n");
	printf("\t...\n");
	printf(" - 'flag' either '-F' or '-L' the first indicate that input will be read from a file while -L indicate that 'input' will be the input string\n");
	printf(" - 'input' is a string or file that the parser will be run on\n");
	exit(1);
}

int main(int argc, char *argv[]) {
	clock_t ticks = clock();
	switch (argc) {
		case 4:
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
	struct rule_arr rule_arr = {
		.rules = malloc(32 * sizeof(rule)),
		.rule_size = 0,
		.rules_alloc_size = 32,
	};

	create_grammer(&rule_arr, grammer_file);

	fclose(grammer_file);
	grammer_file = NULL;

	//find first
	create_first(rule_arr);

	//find follow
	create_follow(rule_arr);

#ifdef DEBUG
	if(rule_arr.rules_alloc_size == rule_arr.rule_size + 1) {
		rule_arr.rules_alloc_size *= 2;
		rule_arr.rules = realloc(rule_arr.rules, rule_arr.rules_alloc_size);
		assert(rule_arr.rules);
	}
	rule_arr.rules[rule_arr.rule_size].name = "special_rule1";
	rule_arr.rules[rule_arr.rule_size + 1].name = "special_rule2";

	print_rules(&rule_arr);
#endif
	
	uint16_t r_alloc_size = 256;
	char* input;
	uint32_t input_size = 0;
	FILE* input_file;
	uint8_t free_input = 0;

	if(argv[2][0] != '-') {
		printf("The second argument must be a flag hence should start with '-' but starts with %c\n", argv[2][0]);
	}
	switch(argv[2][1]) {
		case('F'):
		case('f'):
			free_input = 1;
			input_file = fopen(argv[3], "r");
			assert(input_file);

			fseek(input_file, 0, SEEK_END);
			input_size = ftell(input_file) - 1;
			fseek(input_file, 0, SEEK_SET);
			input = malloc(input_size + 1);

			char c;
			uint32_t i = 0;
			while((c = fgetc(input_file)) != EOF) {
				input[i++] = c;
			}
			input[input_size] = '\0';

			fclose(input_file);
			break;
		case('L'):
		case('l'):
			input = argv[3];
			input_size = strlen(input);
			break;

		default:
			printf("Invalid flag\n");
			exit(1);
	}


	struct rule_info rule_info = { .rule_arr = rule_arr, .rule = 0, .alternative_start_idx = 0, .alternative_end_idx = 0 };
	struct input_info input_info = { .input = input, .input_idx = 0, .input_size = input_size }; 
	struct gss_info gss_info = {
		.gss = init_gss(rule_arr.rule_size, input_size),
	};

	struct set_info set_info = {
		.R_set = init_descriptor_set(r_alloc_size),
		.lesser_input_idx = 0,
		.r_size = 0,
		.r_lower_idx = r_alloc_size >> 1,
		.r_higher_idx = r_alloc_size >> 1,
		.r_alloc_size = r_alloc_size,
	};

	int res = base_loop(&rule_info, &input_info, &gss_info, &set_info);
	printf("------------------\nParsing result: %d\n------------------\n", res);

	ticks = clock() - ticks;
	printf("Time taken %ld clock ticks, %lf ms\n", ticks, ((double) ticks) * 1000/ CLOCKS_PER_SEC);

	if(free_desc_set(set_info.R_set)) {
		printf("failed to free R_set likely a memory leak\n");
	}
	set_info.R_set = NULL;
	if(free_gss(gss_info.gss, rule_arr.rule_size, input_size)) {
		printf("failed to free gss likely a memory leak\n");
	}
	gss_info.gss = NULL;
	if(free_rules(rule_arr)) {
		printf("failed to free rules likely a memory leak\n");
	}
	if(free_input) free(input);
	return 0;
}
