#ifndef DESCRIPTOR_SET_FUNCTIONS
#define DESCRIPTOR_SET_FUNCTIONS

#include <stdint.h>
#include "info_struct.h"

typedef enum {RSET, USET, PSET} set_type;

int print_set_info(const struct rule rules[], struct set_info* set_info, struct input_info* input_info, struct gss_info* gss_info, const uint8_t rule_count);

int in_set(
		const struct rule_info* rule_info,
		const u_descriptors U_set[],
		const uint16_t u_lower_idx,
		const uint16_t u_higher_idx,
		const uint16_t u_alloc_size,
		const uint32_t lesser_input_idx,
		uint32_t input_idx,
		gss_node_idx gss_node_idx,
		uint8_t label_type
		);

int add_descriptor(
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		struct set_info* set_info,
		struct gss_info* gss_info,
		uint8_t label_type
		);

int clean_lesser_from_U(struct gss_info* gss_info, const struct input_info* input_info, const uint32_t lesser_input_idx);

int add_p_set_entry(
		struct set_info* set_info,
		struct gss_info* gss_info,
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		uint32_t input_idx
		);

int add_descriptor_for_P_set(
		const struct rule_info* rule_info,
		const struct input_info* input_info,
		const gss_node_idx new_node,
		const uint32_t new_edge,
		struct gss_info* gss_info,
		struct set_info* set_info
		);

descriptors* init_descriptor_set(const uint16_t size);
u_descriptors* init_u_descriptor_set(const uint16_t size);
int free_desc_set(descriptors* set);
int free_u_desc_set(descriptors* set);

int realloc_r_set(struct set_info* set_info);
int realloc_u_set(struct gss_info* gss_info, const struct rule_info* rule_info, const struct input_info* input_info, const gss_node_idx gss_node_idx, const uint32_t lesser_input_idx);

#endif
