#ifndef BINARIZED_SPPF
#define BINARIZED_SPPF

#include <stdint.h>

enum sppf_node_type {SYMBOL, INTERMEDIAT, TERMINAL};

const uint16_t SSPF_NODE_LIST_SIZE = 256;
const uint16_t PACKED_NODE_LIST_SIZE = 256;

typedef struct {
	uint16_t* children;
	char* block;
	uint16_t block_idx;
	uint16_t left_extent;
	uint16_t right_extent;
	enum sppf_node_type sppf_node_is; 
} sppf_node;

struct packed_nodes {
	uint16_t* children;
	char* block;
	uint16_t block_point;
	uint16_t pivot;
};

int getNodeT(sppf_node* c_n, char t, int i, int j);

int getNodeP(sppf_node* c_n, char* block, int block_idx, int w, int z);

#endif
