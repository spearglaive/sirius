#pragma once
#include <sirius/core/asset_heap_config_table.hpp>
#include <sirius/core/asset_heap_config.hpp>

#include "./buffer_config_table.hpp"

namespace asset_heap_id {
enum : acma::asset_heap_key_t {
	start = buffer_id::num_buffer_ids,

	textures,
	samplers,

	num_total_ids,
	num_asset_heap_ids = num_total_ids - start - 1,
};
}

constexpr acma::asset_heap_config_table<asset_heap_id::num_asset_heap_ids> asset_heap_configs{{{
	{asset_heap_id::textures, acma::asset_heap_config{acma::memory_policy::gpu_local, acma::coupling_policy::coupled, acma::asset_usage_policy::sampled_image, acma::shader_stage::all_graphics}},

	{asset_heap_id::samplers, acma::asset_heap_config{acma::memory_policy::gpu_local, acma::coupling_policy::coupled, acma::asset_usage_policy::sampler, acma::shader_stage::fragment}},
}}};
