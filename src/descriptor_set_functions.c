#include<assert.h>
#include<stdint.h>
#include <stdlib.h>
#include"descriptor_set_functions.h"
#include"gss.h"
#include"binarized_sppf.h"

uint8_t descriptor_hash(descriptors* to_hash) {
	assert(to_hash);
	return to_hash->id & (HASHSET_SIZE - 1);
}

int in_set(descriptors_linked_list* U_set[], descriptors* item) {
	assert(item);
	descriptors_linked_list* ll_pointer = U_set[descriptor_hash(item)];

	if(!ll_pointer) return 0;
	while(ll_pointer) {
		if(item->id == ll_pointer->descriptors->id) return 1;
		ll_pointer = ll_pointer->next;
	}
	return 0;
}

int add_ll_entry(descriptors_linked_list* curr_ll_node, descriptors* item){
	if(curr_ll_node) {
		curr_ll_node->next = (descriptors_linked_list*) malloc(sizeof(descriptors_linked_list));
		curr_ll_node = curr_ll_node->next;
	} else {
		curr_ll_node = (descriptors_linked_list*) malloc(sizeof(descriptors_linked_list));
	}
	if(!curr_ll_node) return 1;
	curr_ll_node->descriptors = item;
	curr_ll_node->next = NULL;
	return 0;
}

int add_descriptor(descriptors_linked_list* R_set[], descriptors_linked_list* U_set[], descriptors* item){
	assert(item);
	if(in_set(U_set, item)) return 0;
	descriptors_linked_list* ll_pointer = R_set[descriptor_hash(item)];

	if(!ll_pointer) {
		if(add_ll_entry(NULL, item)) return 1;
	}

	while(ll_pointer->next) ll_pointer = ll_pointer->next;
	if(add_ll_entry(ll_pointer, item)) return 1;
	return 0;
}

descriptors* pop_descriptor(descriptors_linked_list* R_set[]) {
	int i;
	for(i = 0; i < HASHSET_SIZE; i++) if(R_set[i]) break;
	if(R_set[i]) return NULL;
	descriptors_linked_list* ll_pointer = R_set[i];
	descriptors* ret = R_set[i]->descriptors;
	R_set[i] = R_set[i]->next;
	free(ll_pointer);
	return ret;
}

descriptors* create_descriptor(
		char* grammar_slot,
		uint8_t grammar_slot_idx,
		gss_node* gss_node,
		uint32_t in_pos,
		sppf_node* sppf_node
		) {
	assert(gss_node);
	assert(sppf_node);
	static int curr_id = 0;
	descriptors* descriptor = (descriptors*) malloc(sizeof(descriptors));
	if(!descriptor) return NULL;
	descriptor->id = curr_id++;
	descriptor->grammar_slot = grammar_slot;
	descriptor->grammar_slot_idx = grammar_slot_idx;
	descriptor->gss_node = gss_node;
	descriptor->in_pos = in_pos;
	descriptor->sppf_node = sppf_node;
	return descriptor;
}

int free_descriptor(descriptors* descriptor) {
	if(!descriptor) return 1;
	free(descriptor);
	return 0;
}
