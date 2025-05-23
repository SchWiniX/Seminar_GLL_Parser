#ifndef GRAMMER_HANDLER
#define GRAMMER_HANDLER

#include <stdint.h>
#include <stdio.h>
#include "info_struct.h"

enum grammer_read_states { GRS_NEWLINE, GRS_READ_RULE, GRS_READ_ALTERNATIVES, GRS_READ_RULE_IN_ALTERNATIVE };

int print_rules(const struct rule_arr* rule_arr);

int print_rule_info(const struct rule_info* rule_info, uint8_t full);

uint16_t add_rule(struct rule_arr* rule_arr, struct dym_str rule_name);

int add_alternative_to_rule(struct rule_arr* rule_arr, struct dym_str alternative_buff, uint16_t rule_idx);

uint16_t token_to_idx(char* buff);

void idx_to_token(struct dym_str* buff, uint16_t idx);

int handle_special_chars(FILE* grammer_file, char* curr_char_p);

int create_grammer(struct rule_arr* rules, FILE* grammer_file, uint8_t* count_idx);

uint8_t is_in_first_follow(const uint64_t first_follow[2], const uint8_t* nullable, const signed char c, const uint8_t is_follow);

int create_first(struct rule_arr rules);

int create_follow(struct rule_arr rule_arr);

//assumes rules itself is stack allocated only its components are freed
int free_rules(rule rules[]);

int first_follow_test(const struct rule_info* rule_info, const char c);
#endif
