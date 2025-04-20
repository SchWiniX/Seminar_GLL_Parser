#ifndef DESCRIPTOR_SET_FUNCTIONS
#define DESCRIPTOR_SET_FUNCTIONS

#include<stdint.h>
#include"binarized_sppf.h"
#include"gss.h"


typedef struct descriptors {
	uint16_t grammar_rule_idx;
	uint16_t grammar_slot_pos;
	uint16_t in_pos;
	uint16_t gss_node_idx;
	uint16_t sppf_node_idx;
	uint8_t grammar_slot_idx;
} descriptors;

int in_set(
		descriptors U_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_pos,
		uint16_t in_pos,
		uint16_t gss_node_idx,
		uint16_t sppf_node_idx,
		uint8_t grammar_slot_idx
		);

int add_descriptor(
		descriptors R_set[],
		descriptors U_set[],
		uint16_t grammar_rule_idx,
		uint16_t grammar_slot_pos,
		uint16_t id,
		uint16_t in_pos,
		uint16_t gss_node_idx,
		uint16_t sppf_node_idx,
		uint8_t grammar_slot_idx
		);

descriptors* init_set(uint16_t size);

int free_desc_set(descriptors* set);

#endif
