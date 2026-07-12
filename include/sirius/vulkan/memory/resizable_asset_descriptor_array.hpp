#pragma once
#include "sirius/vulkan/memory/resizable_asset_descriptor_array.fwd.hpp"

#include <vector>
#include <streamline/functional/functor/invoke_each_result.hpp>

#include "sirius/core/descriptor_array_config.hpp"
#include "sirius/vulkan/display/image_view.hpp"
#include "sirius/vulkan/memory/descriptor_pool.hpp"
#include "sirius/vulkan/memory/descriptor_set.hpp"
#include "sirius/vulkan/memory/descriptor_set_layout.hpp"
#include "sirius/vulkan/display/image_sampler.hpp"
#include "sirius/vulkan/display/image_view_handle.hpp"
#include "sirius/core/render_process_core.fwd.hpp"


namespace acma::vk::impl {
	template<descriptor_array_config Config, sl::size_t CommandGroupCount>
	class resizable_asset_descriptor_array_base {
	public:
		constexpr static descriptor_array_config config = Config;
		constexpr static VkDescriptorType descriptor_type = vk::descriptor_types[config.usage];
	public:
		constexpr resizable_asset_descriptor_array_base(render_process_core<CommandGroupCount>& proc_core) noexcept :
			_descriptor_set_layout(), _descriptor_pool(), _descriptor_capacity(), _descriptor_set(),
			proc_ptr(std::addressof(proc_core)) {}
	protected:
		constexpr resizable_asset_descriptor_array_base() noexcept = default;
		template<typename> friend struct ::acma::impl::make;
		template<typename, auto> friend struct ::sl::functor::invoke_each_result;


	public:
		constexpr result<void> initialize() noexcept;

	public:
		constexpr render_process_core<CommandGroupCount> const& render_proc_core() const noexcept { return *proc_ptr; }
		constexpr render_process_core<CommandGroupCount>      & render_proc_core()       noexcept { return *proc_ptr; }
	public:
		constexpr descriptor_set        const& set()        const& noexcept { return _descriptor_set; }
		constexpr descriptor_set_layout const& set_layout() const& noexcept { return _descriptor_set_layout; }


	protected:
		template<typename DescriptorT>
		constexpr result<void> update_descriptors(
			std::vector<DescriptorT> const& descriptor_assets
		) noexcept;

	protected:
		descriptor_set_layout _descriptor_set_layout;
		descriptor_pool _descriptor_pool;
		sl::uint32_t _descriptor_capacity;
		descriptor_set _descriptor_set;
	private:
		sl::reference_ptr<render_process_core<CommandGroupCount>> proc_ptr;
	};
}



namespace acma::vk {
	template<descriptor_array_config Config, typename RenderProcessCoreT>
	requires (Config.usage != asset_usage_policy::sampler)
	class resizable_asset_descriptor_array<Config, RenderProcessCoreT> :
		public impl::resizable_asset_descriptor_array_base<Config, sl::remove_cvref_t<RenderProcessCoreT>::command_buffer_count>,
		public std::vector<vk::image_view_handle>
	{
		using base_type = impl::resizable_asset_descriptor_array_base<Config, sl::remove_cvref_t<RenderProcessCoreT>::command_buffer_count>;
	public:
		using base_type::config;
		using base_type::descriptor_type;

	public:
		constexpr result<void> emplace_back(image_view view) noexcept;
	};
}

namespace acma::vk {
	template<descriptor_array_config Config, typename RenderProcessCoreT>
	requires (Config.usage == asset_usage_policy::sampler)
	class resizable_asset_descriptor_array<Config, RenderProcessCoreT> :
		public impl::resizable_asset_descriptor_array_base<Config, sl::remove_cvref_t<RenderProcessCoreT>::command_buffer_count>,
		public std::vector<vk::image_sampler>
	{
		using base_type = impl::resizable_asset_descriptor_array_base<Config, sl::remove_cvref_t<RenderProcessCoreT>::command_buffer_count>;
	public:
		using base_type::config;
		using base_type::descriptor_type;

	public:
		//TODO: create a sampler_info struct
		constexpr result<void> emplace_back(VkSamplerCreateInfo info) noexcept;
	};
}



#include "sirius/vulkan/memory/resizable_asset_descriptor_array.inl"
