#ifndef DESCRIPTOR_SET_FUNCTIONS
#define DESCRIPTOR_SET_FUNCTIONS

#include<stdint.h>
#include"binarized_sppf.h"
#include"gss.h"

const uint8_t HASHSET_SIZE = 128; // must be a power of 2!

typedef struct descriptors {
	uint32_t id;
	char* grammar_slot;
	uint8_t grammar_slot_idx;
	gss_node* gss_node;
	uint32_t in_pos;
	sppf_node* sppf_node;
} descriptors;

typedef struct descriptors_linked_list {
	struct descriptors* descriptors;
	struct descriptors_linked_list* next;
} descriptors_linked_list;

uint8_t descriptor_hash(descriptors* to_hash);

int in_set(descriptors_linked_list* U_set[], descriptors* item);

int add_descriptor(descriptors_linked_list* R_set[], descriptors_linked_list* U_set[], descriptors* item);

descriptors* pop_descriptor(descriptors_linked_list* R_set[]);

descriptors* create_descriptor(
		char* grammar_slot,
		uint8_t grammar_slot_idx,
		gss_node* gss_node,
		uint32_t in_pos,
		sppf_node* sppf_node
		);

int free_descriptor(descriptors* descriptor);

#endif
