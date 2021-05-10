#pragma once

#define ShadowString(Prefix) ShadowString_##Prefix

#define ShadowString_func_append(p_elements) append(p_elements)
#define ShadowString_c_append(p_shadow_string, p_elements) (p_shadow_string).ShadowString_func_append(p_elements)

#define ShadowString_func_insert_array_at(p_elements, p_index) insert_array_at(p_elements, p_index)
#define ShadowString_c_insert_array_at(p_shadow_string, p_elements, p_index) (p_shadow_string).ShadowString_func_insert_array_at(p_elements, p_index)

#define ShadowString_func_erase_array_at(p_index, p_size) erase_array_at(p_index, p_size)
#define ShadowString_c_erase_array_at(p_shadow_string, p_index, p_size) (p_shadow_string).ShadowString_func_erase_array_at(p_elements, p_index, p_size)

#define ShadowString_func_pop_back_array(p_size) pop_back_array(p_size)
#define ShadowString_c_pop_back_array(p_shadow_string, p_size) (p_shadow_string).ShadowString_func_pop_back_array(p_elements, p_size)

#define ShadowString_func_erase_array_at_always(p_index, p_size) erase_array_at_always(p_index, p_size)
#define ShadowString_c_erase_array_at_always(p_shadow_string, p_index, p_size) (p_shadow_string).ShadowString_func_erase_array_at_always(p_elements, p_index, p_size)

#define ShadowString_func_get(p_index) get(p_index)
#define ShadowString_c_get(p_shadow_string, p_index) (p_shadow_string).ShadowString_func_get(p_index)

#define ShadowString_func_get_size() get_size()
#define ShadowString_c_get_size(p_shadow_string) (p_shadow_string).ShadowString_func_get_size()

#define ShadowString_func_get_length() get_length()
#define ShadowString_c_get_length(p_shadow_string) (p_shadow_string).ShadowString_func_get_length()

#define ShadowString_func_clear() clear()
#define ShadowString_c_clear(p_shadow_string) (p_shadow_string).ShadowString_func_clear()

#define ShadowString_func_to_slice() to_slice()
#define ShadowString_c_to_slice(p_shadow_string) (p_shadow_string).ShadowString_func_to_slice()
