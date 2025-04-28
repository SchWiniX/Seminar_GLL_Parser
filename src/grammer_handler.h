#ifndef GRAMMER_HANDLER
#define GRAMMER_HANDLER

#include <stdint.h>
#include <stdio.h>

typedef struct rule {
	uint16_t* block_sizes;
	char* blocks;
	char* first;
	char* follow;
	uint16_t first_size;
	uint16_t follow_size;
	uint16_t number_of_blocks;
	uint8_t temp_val;
	char name;
} rule;

int create_grammar(rule rules[], FILE* grammar_file);

int combine_rule(rule rules[], char rule, uint16_t* block_sizes, char* blocks, uint16_t number_of_blocks);

int is_non_terminal(char character);

int create_first(rule rules[], char rule);

int create_follow(
		rule rules[],
		char rule,
		char* follow_buff,
		uint16_t* follow_size,
		uint16_t* follow_alloc_size,
		uint8_t temp_info[]
		);

int first_follow_test(char* rules, char rule, char c);
#endif
