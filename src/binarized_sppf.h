#ifndef BINARIZED_SPPF
#define BINARIZED_SPPF

#include <stdint.h>

enum sppf_node_type {SYMBOL, INTERMEDIAT, TERMINAL};

typedef struct {
	char name[2];
	int left_extent;
	int right_extent;
	struct packed_nodes** children;
} symbol_node;

typedef struct {
	char* block;
	int block_idx;
	int left_extent;
	int right_extent;
	struct packed_nodes** children;
} intermediat_node;

typedef struct{
	char terminal;
	int left_extent;
	int right_extent;
} terminal_node;

union sppf_node_p_selector{
	symbol_node* s;
	intermediat_node* i;
	terminal_node* t;
};

typedef struct {
	enum sppf_node_type sppf_node_is; 
	union sppf_node_p_selector node;
} sppf_node;

struct packed_nodes {
	char* block;
	int block_point;
	int pivot;
	sppf_node* nodes;
};

int getNodeT(sppf_node* c_n, char t, int i, int j);

int getNodeP(sppf_node* c_n, char* block, int block_idx, int w, int z);

#endif
