#ifndef GRAMMER_HANDLER
#define GRAMMER_HANDLER

#include <stdint.h>
#include <stdio.h>
#include "info_struct.h"

int print_rules(rule rules[]);

int print_rule_info(const struct rule_info* rule_info, uint8_t full);

int create_grammer(rule rules[], FILE* grammer_file);

int combine_rule(
		rule rules[],
		char rule,
		uint16_t* block_sizes,
		char* blocks,
		uint16_t number_of_blocks,
		uint16_t number_of_blocks_arr_size,
		uint16_t block_alloc_size
		);

int is_non_terminal(char character);

int is_in_first_follow(const uint64_t first_follow[2], const signed char c);

int create_first(rule rules[], char rule, uint64_t first[2], uint8_t temp_val[]);

int create_follow(const rule rules[], char rule, uint64_t follow[2], uint8_t temp_info[]);

//assumes rules itself is stack allocated only its components are freed
int free_rules(rule rules[]);

int first_follow_test(const struct rule_info* rule_info, const char c);
#endif
