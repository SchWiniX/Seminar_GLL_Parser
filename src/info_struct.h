#ifndef INFOSTRUCT
#define INFOSTRUCT

#include <stdint.h>

#define GET_GSS_IDX(rule, input_idx, input_size) (rule - 'A') * (input_size + 1) + input_idx
#define GET_GSS_SIZE(rule_count, input_size) (rule_count + 2) * (input_size + 1)

typedef struct rule {
	uint16_t* alternative_sizes;
	char* alternatives;
	uint64_t first[2];
	uint64_t follow[2];
	uint16_t number_of_alternatives;
	char name;
} rule;

struct rule_info {
	rule* rules;
	uint16_t alternative_start_idx;
	uint16_t alternative_end_idx;
	char rule;
};

struct input_info {
	char* input;
	uint32_t input_idx;
	uint32_t input_size;
};

typedef struct gss_node_idx {
	uint32_t input_idx;
	char rule;
} gss_node_idx;

typedef struct gss_edge {
	gss_node_idx target_node;
	uint16_t alternative_start_idx;
	uint16_t alternative_end_idx;
	char rule;
	uint8_t label_type;
} gss_edge;

typedef struct gss_node {
	gss_edge* edge_arr;
	struct descriptors* U_set;
	uint32_t edge_alloc_size;
	uint32_t edge_size;
	uint16_t u_lower_idx;
	uint16_t u_higher_idx;
	uint16_t u_alloc_size;
	uint16_t u_size;
} gss_node;

struct gss_info {
	gss_node* gss;
	gss_node_idx gss_node_idx;
};

typedef struct descriptors {
	uint32_t input_idx;
	gss_node_idx gss_node_idx;
	uint16_t alternative_start_idx;
	uint16_t alternative_end_idx;
	uint8_t label_type;
	char rule;
} descriptors;

typedef struct p_set_entry {
	uint32_t input_idx;
	gss_node_idx gss_node_idx;
} p_set_entry;

struct set_info {
	descriptors* R_set;
	p_set_entry* P_set;
	uint32_t lesser_input_idx;
	uint32_t p_size;
	uint32_t p_lower_idx;
	uint32_t p_higher_idx;
	uint32_t p_alloc_size;
	uint16_t r_size;
	uint16_t r_lower_idx;
	uint16_t r_higher_idx;
	uint16_t r_alloc_size;
};

#endif
