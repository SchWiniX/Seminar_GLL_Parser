#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "grammer_handler.h"
#include "info_struct.h"
#include "debug.h"

const uint16_t init_first_list_size = 16;
const uint16_t init_alternative_size = 64;
const uint16_t init_number_of_alternatives_size = 8;

#ifdef DEBUG
int print_rules(const struct rule_arr* rule_arr){
	for(int i = 0; i < rule_arr->rule_size; i++) {
		printf("-----------------------------\nrule: %s (%d)\n", rule_arr->rules[i].name, rule_arr->rules[i].count_idx);
		printf("number of alternatives: %d:\n", rule_arr->rules[i].number_of_alternatives);
		printf("alternatives_sizes: ");
		printf("%d", rule_arr->rules[i].alternative_sizes[0]);
		for(int j = 1; j <= rule_arr->rules[i].number_of_alternatives; j++) {
			printf(", %d", rule_arr->rules[i].alternative_sizes[j]);
		}
		printf("\nalternatives: ");
		for(int j = 0; j < rule_arr->rules[i].number_of_alternatives; j++) {
			for(int k = rule_arr->rules[i].alternative_sizes[j]; k < rule_arr->rules[i].alternative_sizes[j + 1]; k++) {
				char c;
				char c2 = -1;
				switch (rule_arr->rules[i].alternatives[k]) {
					case(NONTERMCHAR_NUM):
						c = NONTERMCHAR;
						printf("%c-", c);
						while(rule_arr->rules[i].alternatives[++k] != NONTERMCHAR_NUM)
							printf("%d-", rule_arr->rules[i].alternatives[k]);
						printf("%c", c);
						continue;
					case(EMPTYCHAR_NUM):
						c = EMPTYCHAR;
						break;
					case(NONTERMCHAR):
						c = ESCAPE;
						c2 = NONTERMCHAR;
						break;
					case(EMPTYCHAR):
						c = ESCAPE;
						c2 = EMPTYCHAR;
						break;
					case '\a':
						c = ESCAPE;
						c2 = 'a';
						break;
					case '\b':
						c = ESCAPE;
						c2 = 'b';
						break;
					case '\f':
						c = ESCAPE;
						c2 = 'f';
						break;
					case '\n':
						c = ESCAPE;
						c2 = 'n';
						break;
					case '\r':
						c = ESCAPE;
						c2 = 'r';
						break;
					case '\t':
						c = ESCAPE;
						c2 = 't';
						break;
					case '\v':
						c = ESCAPE;
						c2 = 'v';
						break;
					default:
						c = rule_arr->rules[i].alternatives[k];
				}
				printf("%c", c);
				if(c2 != -1) {
					printf("%c", c2);
					c2 = -1;
				}
			}
			if (j != rule_arr->rules[i].number_of_alternatives) printf(", ");
		}
		printf("\n");

		printf("first(%s): { ", rule_arr->rules[i].name);
		for(unsigned char j = 0; j < 128; j++) {
			if(!is_in_first_follow(rule_arr->rules[i].first, (signed char) j)) continue;
			char j2 = -1;
			switch (j) {
				case(EMPTYCHAR_NUM):
						j = EMPTYCHAR;
						break;
					case(NONTERMCHAR):
						j = ESCAPE;
						j2 = NONTERMCHAR;
						break;
					case(EMPTYCHAR):
						j = ESCAPE;
						j2 = EMPTYCHAR;
						break;
					case '\a':
						j = ESCAPE;
						j2 = 'a';
						break;
					case '\b':
						j = ESCAPE;
						j2 = 'b';
						break;
					case '\f':
						j = ESCAPE;
						j2 = 'f';
						break;
					case '\n':
						j = ESCAPE;
						j2 = 'n';
						break;
					case '\r':
						j = ESCAPE;
						j2 = 'r';
						break;
					case '\t':
						j = ESCAPE;
						j2 = 't';
						break;
					case '\v':
						j = ESCAPE;
						j2 = 'v';
						break;
					default:
						break;
			}
			printf("%c ", (signed char) j);
			if(j2 != -1) {
				printf("%c", j2);
				j2 = -1;
			}
		}
		printf(" }\n");

		printf("follow(%s): { ", rule_arr->rules[i].name);
		for(unsigned char j = 0; j < 128; j++) {
			if(!is_in_first_follow(rule_arr->rules[i].follow, (signed char) j)) continue;
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
	for(int i = rule_info->alternative_start_idx; i < rule_info->alternative_end_idx; i++) {
		printf("%c", rule_info->rules[rule_info->rule - 'A'].alternatives[i]);
	}
	printf("\n");
	return 0;
}
#endif

uint16_t add_rule(struct rule_arr* rule_arr, struct dym_str rule_name) {
	for(int i = 0; i < rule_arr->rule_size; i++) {
		if(!strncmp(rule_arr->rules[i].name, rule_name.str, rule_name.str_size)) {// this may cause problems if name contains negative values
			return i;
		}
	}
	if(rule_arr->rules_alloc_size == rule_arr->rule_size) {
		rule_arr->rules_alloc_size *= 2;
		rule_arr->rules = realloc(rule_arr->rules, rule_arr->rules_alloc_size);
		assert(rule_arr->rules);
	}
	rule_arr->rules[rule_arr->rule_size].name = malloc(rule_name.str_size + 1);
	assert(rule_arr->rules[rule_arr->rule_size].name);
	strncpy(rule_arr->rules[rule_arr->rule_size].name, rule_name.str, rule_name.str_size + 1);// this may cause problems if name contains negative values
	rule_arr->rules[rule_arr->rule_size].name[rule_name.str_size] = '\0';
	rule_arr->rules[rule_arr->rule_size].count_idx = rule_arr->rule_size;
	rule_arr->rules[rule_arr->rule_size].alternative_sizes = NULL;
	rule_arr->rules[rule_arr->rule_size].alternatives = NULL;
	rule_arr->rule_size += 1;
	return rule_arr->rule_size - 1;

}

int add_alternative_to_rule(struct rule_arr* rule_arr, struct dym_str alternative_buff, uint16_t rule_idx) {
	assert(rule_arr);
	assert(rule_arr->rules);
	assert(alternative_buff.str);

	if(alternative_buff.str_size == 0) {
		printf("Empty Alternative found. If this was intended please mark it with the empty string char: %c\n", EMPTYCHAR);
		exit(1);
	}
	rule* rules = rule_arr->rules;

	if(rules[rule_idx].alternatives == NULL) {
		rules[rule_idx].alternatives = malloc(alternative_buff.str_size);
		assert(rules[rule_idx].alternatives);

		rules[rule_idx].alternative_sizes = malloc(2 * sizeof(uint16_t));
		assert(rules[rule_idx].alternative_sizes);

		for(int i = 0; i < alternative_buff.str_size; i++) {
			rules[rule_idx].alternatives[i] = alternative_buff.str[i];
		}
		rules[rule_idx].alternative_sizes[0] = 0;
		rules[rule_idx].alternative_sizes[1] = alternative_buff.str_size;
		rules[rule_idx].number_of_alternatives = 1;
		return 0;
	}

	uint16_t rule_alternative_len = rules[rule_idx].alternative_sizes[rules[rule_idx].number_of_alternatives];
	rules[rule_idx].alternatives = 
		realloc(rules[rule_idx].alternatives, rule_alternative_len + alternative_buff.str_size);
	assert(rules[rule_idx].alternatives);

	rules[rule_idx].alternative_sizes =
		realloc(rules[rule_idx].alternative_sizes, (rules[rule_idx].number_of_alternatives + 1) * sizeof(uint16_t));

	assert(rules[rule_idx].alternative_sizes);
	for(int i = 0; i < alternative_buff.str_size; i++) {
		rules[rule_idx].alternatives[rule_alternative_len + i] = alternative_buff.str[i];
	}

	rule_arr->rules[rule_idx].alternative_sizes[rules[rule_idx].number_of_alternatives + 1] =
		rule_arr->rules[rule_idx].alternative_sizes[rules[rule_idx].number_of_alternatives] + alternative_buff.str_size;
	rule_arr->rules[rule_idx].number_of_alternatives = rule_arr->rules[rule_idx].number_of_alternatives += 1;
	return 0;
}

uint16_t is_non_terminal(char* buff) {
	return (buff[0] == NONTERMCHAR_NUM);
}

uint16_t token_to_idx(char* buff) {
	uint16_t offset = 1;
	uint16_t result;
	uint16_t i = 1;
	while(buff[i] != NONTERMCHAR_NUM) {
		result = buff[i] * offset;
		offset *= 128;
		i++;
	}
	return result;
}

void idx_to_token(struct dym_str* buff, uint16_t idx) {
	assert(buff->str);
	do {
		char c = idx % 128;	
		if(buff->str_size == buff->str_alloc_size) {
			buff->str_alloc_size *= 2;
			buff->str = realloc(buff->str, buff->str_alloc_size);
			assert(buff->str);
		}
		buff->str[buff->str_size++] = c;
		idx /= 128;
	} while(idx != 0);
}

int handle_special_chars(FILE* grammer_file, char* curr_char_p) {	
	assert(grammer_file);
	assert(curr_char_p);

	char curr_char = *curr_char_p;
	if(curr_char == NONTERMCHAR || curr_char == ALTERNATIVECHAR || curr_char == EOG) {
		printf("unescaped special char found where is should not be\n");
		exit(1);
	}
	if(curr_char == EMPTYCHAR) {
		*curr_char_p = EMPTYCHAR_NUM;
		return 0;
	}
	if(curr_char == ESCAPE) {
		*curr_char_p = fgetc(grammer_file);
		switch (*curr_char_p) {
			case 'a':
				*curr_char_p = '\a';
				return 1;
			case 'b':
				*curr_char_p = '\b';
				return 1;
			case 'f':
				*curr_char_p = '\f';
				return 1;
			case 'n':
				*curr_char_p = '\n';
				return 1;
			case 'r':
				*curr_char_p = '\r';
				return 1;
			case 't':
				*curr_char_p = '\t';
				return 1;
			case 'v':
				*curr_char_p = '\v';
				return 1;
			default:
				return 1;
		}
	}
	return 0;
}

	
	// X -> a1 | a2 | ... | an for X a nonterminal and ai strings of terminals and nonterminals
	int create_grammer(struct rule_arr* rules, FILE* grammer_file, uint8_t* count_idx) {
		assert(rules);
		assert(grammer_file);
	
		char curr_char;
		struct dym_str read_buff = {
			.str = malloc(64),
			.str_size = 0,
			.str_alloc_size = 64,
		};
		assert(read_buff.str);
	
		//read start symbol
		if((curr_char = fgetc(grammer_file)) != STARTID) {
			printf("grammer files must start with a valid start symbold declaration\n");
			exit(1);
		}
	
		//consume whitespace
		while((curr_char = fgetc(grammer_file)) == ' ' || curr_char == '\t');
	
		do {
			handle_special_chars(grammer_file, &curr_char);
			if(read_buff.str_size == read_buff.str_alloc_size) {
				read_buff.str_alloc_size *= 2;
				read_buff.str = realloc(read_buff.str, read_buff.str_alloc_size);
				assert(read_buff.str);
		}
		read_buff.str[read_buff.str_size] = curr_char;
		read_buff.str_size += 1;
	} while((curr_char = fgetc(grammer_file)) != '\n');

	//resize rules if nessessary
	assert(rules->rule_size == 0);
	if(rules->rules_alloc_size == 0) {
		rules->rules_alloc_size *= 2;
		rules->rules = realloc(rules->rules, rules->rules_alloc_size);
		assert(rules->rules);
	}

	//add the starting rule as rule 0;
	rules->rules[0].name = malloc(read_buff.str_size + 1);
	assert(rules->rules[0].name);
	strncpy(rules->rules[0].name, read_buff.str, read_buff.str_size);
	rules->rules[0].name[read_buff.str_size] = '\0';
	rules->rules[0].count_idx = 0;
	rules->rules[0].alternative_sizes = NULL;
	rules->rules[0].alternatives = NULL;
	rules->rule_size += 1;

	//start reading grammars
	enum grammer_read_states state = GRS_NEWLINE;
	curr_char = fgetc(grammer_file);
	struct dym_str alternative_buff = {
		.str = malloc(init_alternative_size),
		.str_size = 0,
		.str_alloc_size = init_alternative_size,
	};
	assert(alternative_buff.str);
	while(curr_char != EOG && curr_char != EOF) {
		uint16_t curr_rule_idx;
		while(curr_char != EOG) {
			switch (state) {
				case(GRS_NEWLINE):
					if(curr_char != NONTERMCHAR) {
						printf("Tokens must be enclosed in single quotes was %c\n", curr_char);
						exit(1);
					}
					state = GRS_READ_RULE;
					break;
				case(GRS_READ_RULE):
					//read until we read hit the matching 
					read_buff.str_size = 0;
					while(curr_char != NONTERMCHAR) {
						handle_special_chars(grammer_file, &curr_char);
						if(read_buff.str_size == read_buff.str_alloc_size) {
							read_buff.str_alloc_size *= 2;
							read_buff.str = realloc(read_buff.str, read_buff.str_alloc_size);
							assert(read_buff.str);
						}
						read_buff.str[read_buff.str_size] = curr_char;
						read_buff.str_size += 1;
						curr_char = fgetc(grammer_file);
					}
					//consume the NONTERMCHAR
					curr_char = fgetc(grammer_file);

					//eat whitespace
					while(curr_char == ' ' || curr_char == '\t') curr_char = fgetc(grammer_file);

					//check transition symbol
					if(curr_char != '-' || fgetc(grammer_file) != '>') {
						printf("Rule is missing the production symbol '->'\n");
						exit(1);
					}

					//store rule in rules
					curr_rule_idx = add_rule(rules, read_buff);

					state = GRS_READ_ALTERNATIVES;
					break;
				case(GRS_READ_ALTERNATIVES):
					//eat whitespace
					while(curr_char == ' ' || curr_char == '\t') curr_char = fgetc(grammer_file);
					if(curr_char == NONTERMCHAR) {
						state = GRS_READ_RULE_IN_ALTERNATIVE;
						break;
					}
					if(curr_char == ALTERNATIVECHAR) {
						add_alternative_to_rule(rules, alternative_buff, curr_rule_idx);
						alternative_buff.str_size = 0;
						break;
					}
					if(curr_char == '\n') {
						add_alternative_to_rule(rules, alternative_buff, curr_rule_idx);
						alternative_buff.str_size = 0;
						state = GRS_NEWLINE;
						break;
					}
					handle_special_chars(grammer_file, &curr_char);
					if(alternative_buff.str_size == alternative_buff.str_alloc_size) {
						alternative_buff.str_alloc_size *= 2;
						alternative_buff.str = realloc(alternative_buff.str, alternative_buff.str_alloc_size);
						assert(alternative_buff.str);
					}
					alternative_buff.str[alternative_buff.str_size] = curr_char;
					alternative_buff.str_size += 1;
					break;
				case(GRS_READ_RULE_IN_ALTERNATIVE):
					//read until we read hit the matching 
					read_buff.str_size = 0;
					while(curr_char != NONTERMCHAR) {
						handle_special_chars(grammer_file, &curr_char);
						if(read_buff.str_size == read_buff.str_alloc_size) {
							read_buff.str_alloc_size *= 2;
							read_buff.str = realloc(read_buff.str, read_buff.str_alloc_size);
							assert(read_buff.str);
						}
						read_buff.str[read_buff.str_size] = curr_char;
						read_buff.str_size += 1;
						curr_char = fgetc(grammer_file);
					}
					uint16_t new_rule_idx = add_rule(rules, read_buff);

					//add the rule to the alternative
					if(alternative_buff.str_size == alternative_buff.str_alloc_size) {
						alternative_buff.str_alloc_size *= 2;
						alternative_buff.str = realloc(alternative_buff.str, alternative_buff.str_alloc_size);
						assert(alternative_buff.str);
					}
					alternative_buff.str[alternative_buff.str_size++] = NONTERMCHAR_NUM;

					idx_to_token(&alternative_buff, new_rule_idx);

					if(alternative_buff.str_size == alternative_buff.str_alloc_size) {
						alternative_buff.str_alloc_size *= 2;
						alternative_buff.str = realloc(alternative_buff.str, alternative_buff.str_alloc_size);
						assert(alternative_buff.str);
					}
					alternative_buff.str[alternative_buff.str_size++] = NONTERMCHAR_NUM;

					state = GRS_READ_ALTERNATIVES;
					break;
			}
			curr_char = fgetc(grammer_file);
		}
	}
	
	free(alternative_buff.str);
	free(read_buff.str);
	return 0;
}

uint8_t is_in_first_follow(const uint64_t first_follow[2], const uint8_t* nullable, const signed char c, const uint8_t is_follow) {
	assert(first_follow);

	signed char c_temp = c;
	if(c == EMPTYCHAR_NUM && !is_follow)
		return *nullable;
	else if (c == EMPTYCHAR_NUM)
		return 0;

	int res = 0;
	if(c_temp < 64) {
		res = (first_follow[0] & (uint64_t) 1 << c_temp) >> c_temp;
	} else {
		res = (first_follow[1] & (uint64_t) 1 << (c_temp - 64)) >> (c_temp - 64);
	}
	return res;
}

int add_to_first_follow(uint64_t first_follow[2], uint8_t* nullable, const signed char c, const uint8_t is_follow) {
	assert(first_follow);
	signed char c_temp = c;
	if(c == EMPTYCHAR_NUM && !is_follow) {
		*nullable = 1;
		return 0;
	}

	if(c_temp < 64) {
		first_follow[0] |= (uint64_t) 1 << c_temp;
	} else {
		first_follow[1] |= (uint64_t) 1 << (c_temp - 64);
	}
	return 0;
}

int create_first(struct rule_arr rule_arr) {
	uint8_t has_changed = 1;
	while(has_changed) {
		has_changed = 0;
		for(int i = 0; i < rule_arr.rule_size; i++) {
			uint64_t old_first[2];
			old_first[0] = rule_arr.rules[i].first[0];
			old_first[1] = rule_arr.rules[i].first[1];
			for(int j = 0; j < rule_arr.rules[i].number_of_alternatives; j++) {
				uint8_t finished_nullable = 1;
				for(int k = rule_arr.rules[i].alternative_sizes[j]; k < rule_arr.rules[i].alternative_sizes[j + 1]; k++) {
					if(rule_arr.rules[i].alternatives[k] == EMPTYCHAR_NUM && k == rule_arr.rules[i].alternative_sizes[j]) {
						add_to_first_follow(rule_arr.rules[i].first, EMPTYCHAR_NUM);
					} else if (rule_arr.rules[i].alternatives[k] == EMPTYCHAR_NUM) {
						continue;
					}
					if(!is_non_terminal(rule_arr.rules[i].alternatives + k)) {
						printf("%d adding %c\n", i, rule_arr.rules[i].alternatives[k]);
						add_to_first_follow(rule_arr.rules[i].first, rule_arr.rules[i].alternatives[k]);
						finished_nullable = 0;
						break;
					}
					//add first set of this nonterminal
					uint16_t rule_idx = token_to_idx(rule_arr.rules[i].alternatives + k);
					if(i == rule_idx) break;
					printf("%d adding first of %d\n", i, rule_idx);
					rule_arr.rules[i].first[0] |= (rule_arr.rules[rule_idx].first[0] & (UINT64_MAX - 1)); //exclude null
					rule_arr.rules[i].first[1] |= (rule_arr.rules[rule_idx].first[1]);
					//if nullable continue else return
					if(!is_in_first_follow(rule_arr.rules[i].first, EMPTYCHAR_NUM)) {
						finished_nullable = 0;
						break;
					}
				}
				//when everything was nullable add null
				if(finished_nullable) {
					rule_arr.rules[i].first[0] |= 1;
				}
			}
			has_changed = has_changed || ((old_first[0] ^ rule_arr.rules[i].first[0]) > 0);
			has_changed = has_changed || ((old_first[1] ^ rule_arr.rules[i].first[1]) > 0);
			printf("%d: %d\n", i, has_changed);
		}
	}
	return 0;
}

int create_follow(struct rule_arr rule_arr) {
	uint8_t has_changed = 1;
	while(has_changed) {
	
	}
	return 0;
}
//
//int free_rules(rule rules[]) {
//	int res = 0;
//	for(int i = 0; i < 26; i++) {
//		if(rules[i].name != i + 'A') continue;
//		if(rules[i].alternative_sizes) {
//			free(rules[i].alternative_sizes);
//			rules[i].alternative_sizes = NULL;
//		} else res += 1;
//		if(rules[i].alternatives) {
//			free(rules[i].alternatives);
//			rules[i].alternatives= NULL;
//		} else res += 2;
//	}
//	return res;
//}
//
//int first_follow_test(const struct rule_info* rule_info, const char c) {
//
//	assert(rule_info);
//	assert(rule_info->rules);
//	assert(is_non_terminal(rule_info->rule));
//	assert(rule_info->alternative_start_idx < rule_info->alternative_end_idx);
//
//	struct rule this_rule = rule_info->rules[rule_info->rule - 'A'];
//	int eps_found = 0;
//	int do_continue = 0;
//	for(int alternative_start_idx = rule_info->alternative_start_idx; alternative_start_idx < rule_info->alternative_end_idx; alternative_start_idx++) {
//		if(!is_non_terminal(this_rule.alternatives[alternative_start_idx])) {
//			if(this_rule.alternatives[alternative_start_idx] == '_') {
//				eps_found = 1;
//				do_continue = 1;
//			} else if(this_rule.alternatives[alternative_start_idx] == c) {
//				return 1;
//			}
//		} else {
//			struct rule sub_rule = rule_info->rules[this_rule.alternatives[alternative_start_idx] - 'A'];
//			if(is_in_first_follow(sub_rule.first, c)) {
//				return 1;
//			} else if(is_in_first_follow(sub_rule.first, '_')) {
//				eps_found = 1;
//				do_continue = 1;
//			}
//		}
//		if(!do_continue) break;
//		do_continue = 0;
//	}
//	if(!eps_found) return 0;
//	return is_in_first_follow(this_rule.follow, c);
//}
