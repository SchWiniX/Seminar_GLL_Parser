#include <stdint.h>

enum node_type {SYMBOL, INTERMEDIAT, TERMINAL};

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

union node_p_selector{
	symbol_node* s;
	intermediat_node* i;
	terminal_node* t;
};

typedef struct {
	node_type node_is; 
	node_p_selector node;
} node;

struct packed_nodes {
	char* block;
	int block_point;
	int pivot;
	node* nodes;
};

void getNodeT(node* c_n, char t, int i, int j);

void getNodeP(node* c_n, char* block, int block_idx, int w, int z);
