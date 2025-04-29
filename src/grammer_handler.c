#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "grammer_handler.h"

const uint16_t init_first_list_size = 16;
const uint16_t init_block_size = 32;
const uint16_t init_number_of_blocks_arr_size = 8;

int is_non_terminal(char character) {
	return character >= 65 && character <= 90;
}

int combine_rule(rule rules[], char rule, uint16_t* block_sizes, char* blocks, uint16_t number_of_blocks) {

	assert(rule);
	assert(block_sizes);
	assert(blocks);

	uint8_t idx = ((uint8_t) rule) - 65;
	if(rules[idx].name != rule) {
		rules[idx].name = rule;
		rules[idx].blocks = blocks;
		rules[idx].block_sizes = block_sizes;
		rules[idx].number_of_blocks = number_of_blocks;
		return 0;
	}
	//pain
	rules[idx].blocks = (char*) realloc(rules[idx].blocks, rules[idx].block_sizes[rules[idx].number_of_blocks] + block_sizes[number_of_blocks]);
	assert(rules[idx].blocks);
	rules[idx].block_sizes= (uint16_t*) realloc(rules[idx].block_sizes, (rules[idx].number_of_blocks + number_of_blocks) * sizeof(uint16_t));
	assert(rules[idx].block_sizes);

	for(int i = 0; i < block_sizes[number_of_blocks]; i++) {
		rules[idx].blocks[i + rules[idx].block_sizes[rules[idx].number_of_blocks]] = blocks[i];
	}

	uint16_t curr_rule_total_block_size = rules[idx].block_sizes[rules[idx].number_of_blocks];
	for(int i = 0; i <= number_of_blocks; i++) {
		rules[idx].block_sizes[i + rules[idx].number_of_blocks] = block_sizes[i] + curr_rule_total_block_size;
	}

	rules[idx].number_of_blocks += number_of_blocks;
	free(blocks);
	free(block_sizes);
	return 1;
}

// Assumes grammar file contains only well-formated grammars of type
// X -> a1 | a2 | ... | an for X a nonterminal and ai strings of terminals and nonterminals
int create_grammar(rule rules[], FILE* grammar_file) {
	assert(rules);
	assert(grammar_file);

	uint8_t is_processing_rule = 0;
	uint16_t block_size = 0;
	uint16_t number_of_blocks = 1;
	uint16_t block_alloc_size = init_block_size;
	uint16_t number_of_blocks_arr_size = init_number_of_blocks_arr_size;
	char* block_buff;
	uint16_t* block_size_buff;
	char arrow_buff[5];
	char curr_char = '\0';
	char name;

	while((curr_char = fgetc(grammar_file)) != EOF) {
		if(!is_processing_rule){
			if(!is_non_terminal(curr_char)) {
				printf("faulty formatting found terminating rule_file read early\n");
				return 1;
			}
			name = curr_char;
			fgets(arrow_buff, 4, grammar_file);
			arrow_buff[4] = '\0';
			if(strncmp(arrow_buff, " ->", 5)) {
				printf("faulty formatting found terminating rule_file read earlys\n");
				return 1;
			}
			is_processing_rule = 1;

			block_size = 0;
			block_buff = (char*) malloc(block_alloc_size);
			assert(block_buff);

			number_of_blocks = 1;
			block_size_buff = (uint16_t*) malloc(number_of_blocks_arr_size);
			assert(block_size_buff);
			block_size_buff[0] = 0;
			continue;
		} else {
			switch (curr_char) {
				case ' ': 
					break;
				case '|' :
					if(number_of_blocks >= number_of_blocks_arr_size) {
						number_of_blocks_arr_size *= 2;
						block_size_buff = (uint16_t*) realloc(block_size_buff, number_of_blocks_arr_size);
						assert(block_size_buff);
					}
					block_size_buff[number_of_blocks++] = block_size;
					break;
				case '\n':
					if(number_of_blocks >= number_of_blocks_arr_size) {
						number_of_blocks_arr_size *= 2;
						block_size_buff = (uint16_t*) realloc(block_size_buff, number_of_blocks_arr_size);
						assert(block_size_buff);
					}
					block_size_buff[number_of_blocks] = block_size;
					combine_rule(rules, name, block_size_buff, block_buff, number_of_blocks);
					is_processing_rule = 0;
					break;
				case EOF:
					if(number_of_blocks >= number_of_blocks_arr_size) {
						number_of_blocks_arr_size *= 2;
						block_size_buff = (uint16_t*) realloc(block_size_buff, number_of_blocks_arr_size);
						assert(block_size_buff);
					}
					block_size_buff[number_of_blocks] = block_size;
					combine_rule(rules, name, block_size_buff, block_buff, number_of_blocks);
					return 0;
				default:
					if(block_size >= block_alloc_size) {
						block_alloc_size *= 2;
						block_buff = (char*) realloc(block_buff, block_alloc_size);
						assert(block_buff);
					}
					block_buff[block_size++] = curr_char;
					break;
			}
		}
	}
	return 0;
}

int is_in(char* buff, uint16_t buff_size, char c) {
	assert(buff);

	//char str[buff_size + 1];
	//snprintf(str, buff_size + 1, "%s", buff);
	//printf("checking if %c is in %s\n", c, str);
	for(int i = 0; i < buff_size; i++) {
		if(buff[i] == c) return 1;
	}
	return 0;
}


int create_first(rule rules[], char rule) {
	assert(rules);

	uint16_t rule_idx = rule - 65;
	rules[rule_idx].temp_val = 1;
	uint8_t is_nullable = 0;

	char* first_buff = (char*) malloc(init_first_list_size);
	assert(first_buff);

	uint16_t first_size = 0;
	uint16_t first_alloc_size = init_first_list_size;

	for(int j = 1; j <= rules[rule_idx].number_of_blocks; j++) {
		uint16_t start_idx = rules[rule_idx].block_sizes[j-1];
		uint16_t end_idx = rules[rule_idx].block_sizes[j];

		char curr_char = '\0';
		for(; start_idx < end_idx; start_idx++) {
			if(rules[rule_idx].blocks[start_idx] != '_') {
				curr_char = rules[rule_idx].blocks[start_idx];
				if(
					!is_non_terminal(curr_char) ||
					rules[curr_char - 65].first != NULL ||
					rules[curr_char - 65].temp_val != 1
				) break;
			}
		}
		if(start_idx == end_idx) { //the rule is just the empty string
			if(!is_in(first_buff, first_size, '_') && curr_char == '\0') {
				if(first_size >= first_alloc_size) {
					first_alloc_size *= 2;
					first_buff = (char*) realloc(first_buff, first_alloc_size);
					assert(first_buff);
				}
				//printf("add %c to first of %c\n", '_', rule);
				first_buff[first_size++] = '_';
				is_nullable = 1;
			}
		} else if(!is_non_terminal(curr_char)) {
			if(!is_in(first_buff, first_size, curr_char)) {
				if(first_size >= first_alloc_size) {
					first_alloc_size *= 2;
					first_buff = (char*) realloc(first_buff, first_alloc_size);
					assert(first_buff);
				}
				//printf("add %c to first of %c\n", curr_char, rule);
				first_buff[first_size++] = curr_char;
				is_nullable = 0;
			}
		} else {
			if(rules[curr_char - 65].first == NULL) {
				int res = create_first(rules, curr_char);
				while(res == 1 && start_idx < end_idx - 1) {
					is_nullable = 1;
					res = create_first(rules, rules[rule_idx].blocks[++start_idx]);
				}
			}
			
			if(first_alloc_size < first_size + rules[curr_char - 65].first_size) {
				first_alloc_size = first_size + rules[curr_char - 65].first_size;
				first_buff = (char*) realloc(first_buff, first_alloc_size);
			}
			
			int counter = 0;
			for(int i = first_size; i < first_size + rules[curr_char - 65].first_size; i++) {
				if(is_in(first_buff, first_size, rules[curr_char - 65].first[i - first_size])) continue;
				first_buff[first_size + counter] = rules[curr_char - 65].first[i - first_size];
				counter += 1;
				//printf("added %c from rule %c to %c\n", first_buff[counter - 1], curr_char, rule);
			}
			first_size += counter;
		}
	}
	rules[rule_idx].first = first_buff;
	rules[rule_idx].first_size = first_size;
	return is_nullable;
}

int follow_first(
		rule rules[],
		char rule,
		char* follow_buff,
		uint16_t* follow_size,
		uint16_t* follow_alloc_size,
		char c,
		char origin_rule,
		uint16_t block_idx,
		uint16_t block_size,
		uint8_t temp_info[]
		) { //naming goes brrr

	//printf("follow_first(rule=%c, c=%c)\n", rule, c);
	assert(rule);
	assert(follow_buff);
	assert(follow_size);
	assert(follow_alloc_size);
	assert(is_non_terminal(rule));

	if(!is_non_terminal(c)) {
		if(*follow_size >= *follow_alloc_size) {
			(*follow_alloc_size) *= 2;
			follow_buff = (char*) realloc(follow_buff, *follow_alloc_size);
			assert(follow_buff);
		}
		if(is_in(follow_buff, *follow_size, c)) return 0;
		//printf("adding %c to follow(%c)\n", c, rule);
		follow_buff[(*follow_size)++] = c;
		return 0;
	} 
	struct rule sub_rule = rules[c - 65];

	for(uint16_t i = 0; i < sub_rule.first_size; i++) {
		if (sub_rule.first[i] != '_') {
			if(is_in(follow_buff, *follow_size, sub_rule.first[i])) continue;

			if(*follow_size >= *follow_alloc_size) {
				*follow_alloc_size *= 2;
				follow_buff = (char*) realloc(follow_buff, *follow_alloc_size);
				assert(follow_buff);
			}
			//printf("adding %c to follow(%c) from first(%c)\n", sub_rule.first[i], rule, c);
			follow_buff[(*follow_size)++] = sub_rule.first[i];
			continue;
		}
		if(block_idx == block_size - 1 && temp_info[i - 65] != 1) {
			create_follow(rules, origin_rule, follow_buff, follow_size, follow_alloc_size, temp_info);
		} else {
			follow_first(
					rules,
					rule,
					follow_buff,
					follow_size,
					follow_alloc_size,
					sub_rule.blocks[block_idx + 1],
					origin_rule,
					block_idx + 1,
					block_size,
					temp_info
					);
		}
	}
	return 0;
}

int create_follow(
		rule rules[],
		char rule,
		char* follow_buff,
		uint16_t* follow_size,
		uint16_t* follow_alloc_size,
		uint8_t temp_info[]
		) {
	
	//printf("create_follow(%c)\n", rule);
	assert(rule);
	assert(follow_buff);
	assert(follow_size);
	assert(follow_alloc_size);
	assert(is_non_terminal(rule));

	temp_info[rule - 65] = 1;
	if(rule == 'S') {
		follow_buff[(*follow_size)++] = '$';
	}

	for(int i = 0; i < 26; i++) {
		if(rules[i].name != i + 65) continue;
		struct rule sub_rule = rules[i];
		for(int j = 1; j <= sub_rule.number_of_blocks; j++) {
			uint16_t start_idx = sub_rule.block_sizes[j-1];
			uint16_t end_idx = sub_rule.block_sizes[j];

			for(; start_idx < end_idx - 1; start_idx++) {
				if(sub_rule.blocks[start_idx] != rule) continue;
				follow_first(
						rules,
						rule,
						follow_buff,
						follow_size,
						follow_alloc_size,
						sub_rule.blocks[start_idx + 1],
						sub_rule.name,
						start_idx + 1,
						end_idx,
						temp_info
						);
			}
			if(sub_rule.blocks[end_idx - 1] == rule && sub_rule.name != rule && temp_info[i] != 1) {
				create_follow(rules, sub_rule.name, follow_buff, follow_size, follow_alloc_size, temp_info);
			}
		}
	}
	return 0;
}
