#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

//#include "descriptor_set_functions.h"
#include "grammer_handler.h"
//#include "gss.h"
//#include "gll_parser.h"
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
	struct rule_arr rule_arr = {
		.rules = malloc(32 * sizeof(rule)),
		.rule_size = 0,
		.rules_alloc_size = 32,
	};

	uint8_t rule_count = 0; //amount of rules in this calc
	create_grammer(&rule_arr, grammer_file, &rule_count);


	//find first
	create_first(rule_arr);

	print_rules(&rule_arr);
	exit(1);
//
//	//find follow
//	for(int i = 0; i < 26; i++) {
//		if(rules[i].name == i + 65) {
//			for(int i = 0; i < 26; i++) {
//				temp_vals[i] = 0;
//			}		
//			create_follow(rules, i + 65, rules[i].follow, temp_vals);
//		}
//	}
//	fclose(grammer_file);
//	grammer_file = NULL;
//
//	rule_count += 1;
//	rules[26].name = 91;
//	rules[26].first[0] = 0;
//	rules[26].first[1] = 0;
//	rules[26].follow[0] = 0;
//	rules[26].follow[1] = 0;
//	rules[26].count_idx = rule_count;
//
//	rules[27].name = 92;
//	rules[27].first[0] = 0;
//	rules[27].first[1] = 0;
//	rules[27].follow[0] = 0;
//	rules[27].follow[1] = 0;
//	rules[27].count_idx = rule_count + 1;
//
//#ifdef DEBUG
//	print_rules(rules);
//#endif
//
//	uint16_t r_alloc_size = 256;
//
//	uint32_t input_size = strlen(argv[2]); //this feels hella unsafe... anyway moving on
//
//	struct rule_info rule_info = { .rules = rules, .rule = 'S', .alternative_start_idx = 0, .alternative_end_idx = 0 };
//	struct input_info input_info = { .input = argv[2], .input_idx = 0, .input_size = input_size }; 
//	struct gss_info gss_info = {
//		.gss = init_gss(rule_count, input_size),
//	};
//
//	struct set_info set_info = {
//		.R_set = init_descriptor_set(r_alloc_size),
//		.lesser_input_idx = 0,
//		.r_size = 0,
//		.r_lower_idx = r_alloc_size >> 1,
//		.r_higher_idx = r_alloc_size >> 1,
//		.r_alloc_size = r_alloc_size,
//	};
//
//	int res = base_loop(&rule_info, &input_info, &gss_info, &set_info, rule_count);
//	printf("------------------\nParsing result: %d\n------------------\n", res);
//
//	ticks = clock() - ticks;
//	printf("Time taken %ld clock ticks, %lf ms\n", ticks, ((double) ticks) * 1000/ CLOCKS_PER_SEC);
//
//	if(free_desc_set(set_info.R_set)) {
//		printf("failed to free R_set likely a memory leak\n");
//	}
//	set_info.R_set = NULL;
//	if(free_gss(gss_info.gss, rule_count, input_size)) {
//		printf("failed to free gss likely a memory leak\n");
//	}
//	gss_info.gss = NULL;
//	if(free_rules(rules)) {
//		printf("failed to free rules likely a memory leak\n");
//	}
	return 0;
}
