#ifndef INFOSTRUCT
#define INFOSTRUCT

#include <stdint.h>

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

struct input_info {
	char* input;
	uint32_t input_idx;
	uint32_t input_size;
};

typedef struct gss_node{
	uint16_t rule;
	uint16_t block_idx;
	uint16_t block_end_idx;
	uint32_t input_idx;
	uint8_t label_type;
} gss_node;

typedef struct gss_edge {
	uint16_t src_node;
	uint16_t target_node;
} gss_edge;

struct gss_info {
	gss_node* gss_nodes;
	gss_edge* gss_edges;
	uint16_t gss_node_idx;
	uint16_t gss_node_alloc_array_size;
	uint16_t gss_edge_alloc_array_size;
	uint16_t gss_node_array_size;
	uint16_t gss_edge_array_size;
};

typedef struct descriptors {
	uint32_t input_idx;
	uint16_t block_idx;
	uint16_t block_end_idx;
	uint16_t gss_node_idx;
	uint8_t label_type;
	char rule;
} descriptors;

typedef struct p_set_entry {
	uint32_t input_idx;
	uint16_t gss_node_idx;
} p_set_entry;

struct set_info {
	descriptors* R_set;
	descriptors* U_set;
	p_set_entry* P_set;
	uint16_t r_size;
	uint16_t u_size;
	uint16_t p_size;
	uint16_t r_alloc_size;
	uint16_t u_alloc_size;
	uint16_t p_alloc_size;
};

#endif
