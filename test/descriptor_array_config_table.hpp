#pragma once
#include <sirius/core/descriptor_array_config_table.hpp>
#include <sirius/core/descriptor_array_config.hpp>

#include "./buffer_config_table.hpp"

namespace descriptor_array_id {
enum : acma::descriptor_key_t {
	start = buffer_id::num_buffer_ids,

	textures,
	samplers,

	num_total_ids,
	num_descriptor_array_ids = num_total_ids - start - 1,
};
}

constexpr acma::descriptor_array_config_table<descriptor_array_id::num_descriptor_array_ids> descriptor_array_configs{{{
	{descriptor_array_id::textures, acma::descriptor_array_config{acma::asset_usage_policy::sampled_image, acma::shader_stage::all_graphics}},

	{descriptor_array_id::samplers, acma::descriptor_array_config{acma::asset_usage_policy::sampler, acma::shader_stage::fragment}},
}}};
