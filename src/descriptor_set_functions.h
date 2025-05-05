#ifndef DESCRIPTOR_SET_FUNCTIONS
#define DESCRIPTOR_SET_FUNCTIONS

#include <stdint.h>
#include "info_struct.h"

typedef enum {RSET, USET, PSET} set_type;

int print_set_info(const struct rule rules[], struct set_info* set_info);

int in_set(
		const struct rule_info* rule_info,
		const descriptors U_set[],
		const uint16_t u_lower_idx,
		const uint16_t u_higher_idx,
		const uint16_t u_alloc_size,
		uint32_t input_idx,
		uint32_t gss_node_idx,
		uint8_t label_type
		);

int add_descriptor(
		const struct rule_info* rule_info,
		struct set_info* set_info,
		uint32_t input_idx,
		uint32_t gss_node_idx,
		uint8_t label_type
		);

int clean_lesser_from_U(struct set_info* set_info);

int add_p_set_entry(struct set_info* set_info, uint32_t gss_node_idx, uint32_t input_idx);

descriptors* init_descriptor_set(uint16_t size);
int free_desc_set(descriptors* set);
descriptors* realloc_descriptor_set(struct set_info* set_info, set_type s);

p_set_entry* init_p_set_entry_set(uint32_t size);
int free_p_set_entry_set(p_set_entry* set);
#endif
