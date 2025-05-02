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
	char name;
} rule;

struct rule_info {
	rule* rules;
	uint16_t start_idx;
	uint16_t end_idx;
	char rule;
};

int print_rule_info(struct rule_info* rule_info, uint8_t full);

int create_grammar(rule rules[], FILE* grammar_file);

int combine_rule(rule rules[], char rule, uint16_t* block_sizes, char* blocks, uint16_t number_of_blocks);

int is_non_terminal(char character);

int create_first(rule rules[], char rule, uint8_t temp_val[]);

int create_follow(
		rule rules[],
		char rule,
		char* follow_buff,
		uint16_t* follow_size,
		uint16_t* follow_alloc_size,
		uint8_t temp_info[]
		);

int first_follow_test(
		rule* rules,
		char rule,
		uint16_t block_idx,
		uint16_t block_end_idx,
		char c
		);
#endif
