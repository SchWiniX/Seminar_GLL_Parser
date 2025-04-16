#ifndef DESCRIPTOR_SET_FUNCTIONS
#define DESCRIPTOR_SET_FUNCTIONS

#include<stdint.h>
#include"binarized_sppf.h"
#include"gss.h"

const int ARRAY_SIZE = 64;

typedef struct descriptors {
	char* grammar_slot;
	gss_node* gss_node;
	int in_pos;
	sppf_node* sppf_node;
	//used for linked lists
	struct descriptors* next;
} descriptors;

typedef struct descriptors_linked_list {
	struct descriptors desriptors_arr[ARRAY_SIZE];
	struct descriptors_linked_list* next;
} descriptors_linked_list;

int descriptor_hash(descriptors* to_hash);

int add_descriptor(descriptors_linked_list* linked_list, descriptors* item);

descriptors* pop_descriptor(descriptors_linked_list* linked_list);
#endif
