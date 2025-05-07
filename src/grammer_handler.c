#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "grammer_handler.h"
#include "info_struct.h"
#include "debug.h"

const uint16_t init_first_list_size = 16;
const uint16_t init_block_size = 64;
const uint16_t init_number_of_blocks_arr_size = 8;

#ifdef DEBUG
int print_rules(rule rules[]){
	for(int i = 0; i < 26; i++) {
		if(rules[i].name != (char) (i + 'A')) continue;
		printf("-----------------------------\nrule: %c\n", rules[i].name);
		printf("number of blocks: %d:\n", rules[i].number_of_blocks);
		printf("blocks_sizes: ");
		printf("%d", rules[i].block_sizes[0]);
		for(int j = 1; j <= rules[i].number_of_blocks; j++) {
			printf(", %d", rules[i].block_sizes[j]);
		}
		printf("\nblocks: ");
		for(int j = 1; j <= rules[i].number_of_blocks; j++) {
			for(int k = rules[i].block_sizes[j - 1]; k < rules[i].block_sizes[j]; k++) {
				printf("%c", rules[i].blocks[k]);
			}
			if (j != rules[i].number_of_blocks) printf(", ");
		}
		printf("\n");

		if(rules[i].name != (char) (i + 'A')) continue;
		printf("first(%c): { ", rules[i].name);
		for(unsigned char j = 0; j < 128; j++) {
			if(!is_in_first_follow(rules[i].first, (signed char) j)) continue;
			printf("%c ", (signed char) j);
		}
		printf(" }\n");

		printf("follow(%c): { ", rules[i].name);
		for(unsigned char j = 0; j < 128; j++) {
			if(!is_in_first_follow(rules[i].follow, (signed char) j)) continue;
			printf("%c ", (signed char) j);
		}
		printf(" }\n");
	}
	return 0;
}

int print_rule_info(const struct rule_info* rule_info, uint8_t full) {
	printf("Rule: %c", rule_info->rule);
	if(!full) {
		printf("\n");
		return 0;
	}
	printf(" -> ");
	for(int i = rule_info->start_idx; i < rule_info->end_idx; i++) {
		printf("%c", rule_info->rules[rule_info->rule - 'A'].blocks[i]);
	}
	printf("\n");
	return 0;
}
#endif

int is_non_terminal(char character) {
	return character >= 'A' && character <= 90;
}

int combine_rule(
		rule rules[],
		char rule,
		uint16_t* block_sizes,
		char* blocks,
		uint16_t number_of_blocks,
		uint16_t number_of_blocks_arr_size,
		uint16_t block_alloc_size
		) {

	assert(rule);
	assert(block_sizes);
	assert(blocks);

	uint8_t idx = ((uint8_t) rule) - 'A';
	if(rules[idx].name != rule) {
		rules[idx].name = rule;
		rules[idx].blocks = blocks;
		rules[idx].block_sizes = block_sizes;
		rules[idx].number_of_blocks = number_of_blocks;
		return 0;
	}
	//pain
	uint16_t old_total_block_size = rules[idx].block_sizes[rules[idx].number_of_blocks]; 
	uint16_t new_total_block_size = block_sizes[number_of_blocks];

	//loop through old blocks
	for(int i = 0; i < old_total_block_size; i++) {
		if(new_total_block_size + i >= block_alloc_size) {
			block_alloc_size *= 2;
			blocks = realloc(blocks, block_alloc_size);
			assert(blocks);
		}
		blocks[i + new_total_block_size] = rules[idx].blocks[i];
	}
	free(rules[idx].blocks);
	rules[idx].blocks = blocks;

	for(int i = 1; i <= rules[idx].number_of_blocks; i++) {
		if(number_of_blocks + i >= number_of_blocks_arr_size) {
			number_of_blocks_arr_size *= 2;
			block_sizes = realloc(block_sizes, number_of_blocks_arr_size * sizeof(uint16_t));
			assert(block_sizes);
		}
		block_sizes[i + number_of_blocks] = rules[idx].block_sizes[i] + new_total_block_size;
	}
	free(rules[idx].block_sizes);
	rules[idx].block_sizes = block_sizes;

	rules[idx].number_of_blocks += number_of_blocks;
	return 1;
}

// Assumes grammer file contains only well-formated grammers of type
// X -> a1 | a2 | ... | an for X a nonterminal and ai strings of terminals and nonterminals
int create_grammer(rule rules[], FILE* grammer_file) {
	assert(rules);
	assert(grammer_file);

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

	while((curr_char = fgetc(grammer_file)) != EOF) {
		if(!is_processing_rule){
			if(!is_non_terminal(curr_char)) {
				return 1;
			}
			name = curr_char;
			fgets(arrow_buff, 4, grammer_file);
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
			block_size_buff = (uint16_t*) malloc(number_of_blocks_arr_size * sizeof(uint16_t));
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
						block_size_buff = (uint16_t*) realloc(block_size_buff, number_of_blocks_arr_size * sizeof(uint16_t));
						assert(block_size_buff);
					}
					block_size_buff[number_of_blocks++] = block_size;
					break;
				case '\n':
					if(number_of_blocks + 1>= number_of_blocks_arr_size) {
						number_of_blocks_arr_size *= 2;
						block_size_buff = (uint16_t*) realloc(block_size_buff, number_of_blocks_arr_size * sizeof(uint16_t));
						assert(block_size_buff);
					}
					block_size_buff[number_of_blocks] = block_size;
					combine_rule(rules, name, block_size_buff, block_buff, number_of_blocks, number_of_blocks_arr_size, block_alloc_size);
					is_processing_rule = 0;
					break;
				case EOF:
					if(number_of_blocks + 1 >= number_of_blocks_arr_size) {
						number_of_blocks_arr_size *= 2;
						block_size_buff = (uint16_t*) realloc(block_size_buff, number_of_blocks_arr_size * sizeof(uint16_t));
						assert(block_size_buff);
					}
					block_size_buff[number_of_blocks] = block_size;
					combine_rule(rules, name, block_size_buff, block_buff, number_of_blocks, number_of_blocks_arr_size, block_alloc_size);
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

int is_in_first_follow(const uint64_t first_follow[2], const signed char c) {
	assert(first_follow);

	int res = 0;
	if(c < 64) {
		res = (first_follow[0] & (uint64_t) 1 << c) >> c;
	} else {
		res = (first_follow[1] & (uint64_t) 1 << (c - 64)) >> (c - 64);
	}
	return res;
}

int add_to_first_follow(uint64_t first_follow[2], const signed char c) {
	if(c < 64) {
		first_follow[0] |= (uint64_t) 1 << c;
	} else {
		first_follow[1] |= (uint64_t) 1 << (c - 64);
	}
	return 0;
}


int create_first(rule rules[], char rule, uint64_t first[2], uint8_t temp_val[]) {
	assert(rules);

	uint16_t rule_idx = rule - 'A';
	if(temp_val[rule_idx] == 1) return is_in_first_follow(rules[rule_idx].first, '_');
	temp_val[rule_idx] = 1;
	uint8_t is_nullable = 0;

	for(int j = 0; j < rules[rule_idx].number_of_blocks; j++) {
		uint16_t start_idx = rules[rule_idx].block_sizes[j];
		uint16_t end_idx = rules[rule_idx].block_sizes[j + 1];

		for(int i = start_idx; i < end_idx; i++) {
			char curr_char = rules[rule_idx].blocks[i];
			if(curr_char == '_') {
				add_to_first_follow(first, '_');
				is_nullable = 1;
				break;
			} else if(!is_non_terminal(curr_char)) {
				add_to_first_follow(first, curr_char);
				break;
			} else {
				int res = create_first(rules, curr_char, first, temp_val);
				rules[rule_idx].first[0] |= rules[curr_char - 'A'].first[0];
				rules[rule_idx].first[1] |= rules[curr_char - 'A'].first[1];
				if(!res) break;
			}
		}
	}
	return is_nullable;
}

int follow_first(
		const rule rules[],
		char rule,
		uint64_t follow[2],
		char c,
		char origin_rule,
		uint16_t block_idx,
		uint16_t block_size,
		uint8_t temp_info[]
		) { //naming goes brrr

	assert(rule);
	assert(follow);
	assert(temp_info);
	assert(is_non_terminal(rule));

	if(!is_non_terminal(c)) {
		add_to_first_follow(follow, c);
		return 0;
	} 
	struct rule sub_rule = rules[c - 'A'];

	for(unsigned char i = 0; i < 128; i++) {
		if(!is_in_first_follow(sub_rule.first, i)) continue;
		if((signed char) i != '_') {
			add_to_first_follow(follow, (signed char) i);
			continue;
		}
		if(block_idx == block_size - 1 && temp_info[origin_rule - 'A'] != 1) {
			create_follow(rules, origin_rule, follow, temp_info);
		} else {
			follow_first(
					rules,
					rule,
					follow,
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

int create_follow(const rule rules[], char rule, uint64_t follow[2], uint8_t temp_info[]) {
	
	assert(rule);
	assert(follow);
	assert(temp_info);
	assert(is_non_terminal(rule));

	temp_info[rule - 'A'] = 1;
	if(rule == 'S') {
		add_to_first_follow(follow, '\0');
	}

	for(int i = 0; i < 26; i++) {
		if(rules[i].name != i + 'A') continue;
		struct rule sub_rule = rules[i];
		for(int j = 1; j <= sub_rule.number_of_blocks; j++) {
			uint16_t start_idx = sub_rule.block_sizes[j-1];
			uint16_t end_idx = sub_rule.block_sizes[j];

			for(; start_idx < end_idx - 1; start_idx++) {
				if(sub_rule.blocks[start_idx] != rule) continue;
				follow_first(
						rules,
						rule,
						follow,
						sub_rule.blocks[start_idx + 1],
						sub_rule.name,
						start_idx + 1,
						end_idx,
						temp_info
						);
			}
			if(sub_rule.blocks[end_idx - 1] == rule && sub_rule.name != rule && temp_info[i] != 1) {
				create_follow(rules, sub_rule.name, follow, temp_info);
			}
		}
	}
	return 0;
}

int free_rules(rule rules[]) {
	int res = 0;
	for(int i = 0; i < 26; i++) {
		if(rules[i].name != i + 'A') continue;
		if(rules[i].block_sizes) {
			free(rules[i].block_sizes);
			rules[i].block_sizes = NULL;
		} else res += 1;
		if(rules[i].blocks) {
			free(rules[i].blocks);
			rules[i].blocks= NULL;
		} else res += 2;
	}
	return res;
}

int first_follow_test(const struct rule_info* rule_info, const char c) {

	assert(rule_info);
	assert(rule_info->rules);
	assert(is_non_terminal(rule_info->rule));
	assert(rule_info->start_idx < rule_info->end_idx);

	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
	int eps_found = 0;
	int do_continue = 0;
	for(int start_idx = rule_info->start_idx; start_idx < rule_info->end_idx; start_idx++) {
		if(!is_non_terminal(this_rule.blocks[start_idx])) {
			if(this_rule.blocks[start_idx] == '_') {
				eps_found = 1;
				do_continue = 1;
			} else if(this_rule.blocks[start_idx] == c) {
				return 1;
			}
		} else {
			struct rule sub_rule = rule_info->rules[this_rule.blocks[start_idx] - 'A'];
			if(is_in_first_follow(sub_rule.first, c)) {
				return 1;
			} else if(is_in_first_follow(sub_rule.first, '_')) {
				eps_found = 1;
				do_continue = 1;
			}
		}
		if(!do_continue) break;
		do_continue = 0;
	}
	if(!eps_found) return 0;
	return is_in_first_follow(this_rule.follow, c);
}
