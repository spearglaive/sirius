
#include <bit>
#include <cstdint>
#include <iostream>
#include <set>
#include <array>
#include <string>
#include <type_traits>
#include <utility>
#include <exception>
#include <filesystem>

#include <result/to_result.hpp>

#include <sirius/core/error.hpp>
#include <sirius/core/make.hpp>
#include <sirius/core/initialize.hpp>
#include <sirius/core/window.hpp>
#include <sirius/arith/matrix.hpp>
#include <sirius/arith/point.hpp>
#include "sirius/core/render_instance.hpp"
#include "sirius/core/decoder.hpp"
#include "sirius/input/category.hpp"
#include "sirius/input/code.hpp"
#include "sirius/timeline/command_traits.hpp"
#include "sirius/timeline/predefined_callbacks/update_swap_extent.hpp"

#include "./timeline.hpp"
#include "./buffer_config_table.hpp"
#include "./asset_heap_config_table.hpp"



#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
extern "C" const char* __asan_default_options() { return "detect_leaks=0"; }
#endif



using render_instance = acma::render_instance<acma::test::intermediate_timeline, buffer_configs, asset_heap_configs>;

using command_traits_type = acma::timeline::impl::command_traits<
	acma::test::basic_timeline, 
	0, 
	sl::index_sequence_type<>, 
	sl::integer_sequence_type<acma::command_family_t>
>;

static_assert(command_traits_type::group_indices[0] == 0);
static_assert(command_traits_type::group_indices[1] == 1);
static_assert(command_traits_type::group_indices[2] == 1);
static_assert(command_traits_type::group_indices[3] == 1);
static_assert(command_traits_type::group_indices[4] == 1);
static_assert(command_traits_type::group_indices[5] == 1);
static_assert(command_traits_type::group_indices[6] == 2);
static_assert(command_traits_type::group_indices[7] == 2);

static_assert(command_traits_type::group_families[0] == acma::command_family::none);
static_assert(command_traits_type::group_families[1] == acma::command_family::graphics);
static_assert(command_traits_type::group_families[2] == acma::command_family::present);


constexpr sl::array<6, std::uint16_t> rect_indices{{0, 1, 2, 2, 1, 3}};
constexpr sl::array<3, acma::pt2u32> rect_positions{{
	{{{0, 0}}},
	{{{400, 200}}},
	{{{800, 400}}},
}};

int main(){
	RESULT_VERIFY(acma::intitialize_lib("Sirius Test", acma::version{1,0,0}));


    std::cout << std::filesystem::current_path() << std::endl;
    const std::filesystem::path assets_path = std::filesystem::canonical(std::filesystem::path("../../test/assets"));
	
	acma::vk::physical_device& selected_device = *acma::devices().begin();
    acma::result<render_instance> inst_result = acma::make<render_instance>(selected_device, true, acma::sz2u32{1600, 900});
    if(!inst_result.has_value()) return inst_result.error();
    render_instance inst = *std::move(inst_result);

 
	//llfio::mapped_file_handle mh;
	//RESULT_TRY_MOVE(mh, acma::decoder::open_file(assets_path / "test_font.ttf"));
	//RESULT_VERIFY(acma::decoder::decode_font(mh));


	
	//Set callback to update gpu address
	inst.timeline_callbacks()[acma::timeline::callback_event::on_frame_begin].push_back([](typename render_instance::render_process_type& proc, acma::window& win, auto&) noexcept -> acma::result<void> {
		{
		draw_constants constants {
			.swap_extent = win.swap_chain_ptr()->extent(),
			.position_buff_addr = sl::universal::get<buffer_id::positions>(proc).gpu_address()
		};
		std::memcpy(
			sl::universal::get<buffer_id::draw_constants>(proc).data(),
			&constants,
			sizeof(draw_constants)
		);
		}

		{
		compute_constants constants {
			.buffer_addresses_addr = sl::universal::get<buffer_id::compute_buffer_addresses>(proc).gpu_address()
		};
		std::memcpy(
			sl::universal::get<buffer_id::compute_constants>(proc).data(),
			&constants,
			sizeof(compute_constants)
		);

		sl::array<3, VkDeviceAddress> compute_addresses{{
			sl::universal::get<buffer_id::counts>(proc).gpu_address(),

			sl::universal::get<buffer_id::positions>(proc).gpu_address(),
			sl::universal::get<buffer_id::draw_commands>(proc).gpu_address(),
		}};

		std::memcpy(
			sl::universal::get<buffer_id::compute_buffer_addresses>(proc).data(),
			compute_addresses.data(),
			compute_addresses.size_bytes()
		);
		}
		return {};
	});
	inst.timeline_callbacks()[acma::timeline::callback_event::on_swap_chain_updated].push_back(
		&acma::timeline::predefined_callbacks::update_swap_extent<render_instance, buffer_id::draw_constants>
	);


	auto& staging_buffer = sl::universal::get<buffer_id::staging>(inst);

	//Set dispatch commands
	{
	RESULT_VERIFY((sl::universal::get<buffer_id::dispatch_commands>(inst).template try_emplace_back<VkDispatchIndirectCommand>(
		1, 1, 1
	)));
	}


	//Set initial draw commads
	{
	RESULT_VERIFY(sl::universal::get<buffer_id::draw_commands>(inst).reserve(sizeof(acma::indexed_draw_command_t))); //Should do nothing
	RESULT_VERIFY((sl::universal::get<buffer_id::draw_commands>(inst).template try_emplace_back<acma::indexed_draw_command_t>(
		6, 3, 0, 0, 0
	)));
	RESULT_VERIFY((sl::universal::get<buffer_id::counts>(inst).template try_emplace_back<acma::draw_count_t>(
		1
	)));


	RESULT_VERIFY((staging_buffer.template emplace_back<acma::indexed_draw_command_t>(
		6, 1, 0, 0, 0
	)));

	RESULT_VERIFY(sl::universal::get<buffer_id::single_instance_draw_command>(inst).resize(sizeof(acma::indexed_draw_command_t)));
	RESULT_VERIFY((acma::gpu_copy(
		sl::universal::get<buffer_id::single_instance_draw_command>(inst),
		staging_buffer,
		sizeof(acma::indexed_draw_command_t)
	)));
	staging_buffer.clear();
	}




	//Set rectangle indices
	{
	RESULT_VERIFY(staging_buffer.resize(rect_indices.size_bytes()));
	std::memcpy(staging_buffer.data(), rect_indices.data(), rect_indices.size_bytes());
	
	
	RESULT_VERIFY(staging_buffer.upload_to(sl::universal::get<buffer_id::rectangle_indices>(inst)));

	//Equivalent to:
	// RESULT_VERIFY(sl::universal::get<buffer_id::rectangle_indices>(inst).resize(rect_indices.size_bytes()));

	// RESULT_VERIFY((acma::copy(
	// 	sl::universal::get<buffer_id::rectangle_indices>(inst),
	// 	staging_buffer,
	// 	rect_indices.size_bytes()
	// )));
	// staging_buffer.clear();
	}


	//Set rectangle positions
	{
	RESULT_VERIFY(staging_buffer.resize(rect_positions.size_bytes()));
	std::memcpy(staging_buffer.data(), rect_positions.data(), rect_positions.size_bytes());

	RESULT_VERIFY(sl::universal::get<buffer_id::positions>(inst).resize((sizeof(acma::pt2u32) * 16 * 16) + 1));

	RESULT_VERIFY((acma::gpu_copy(
		sl::universal::get<buffer_id::positions>(inst),
		staging_buffer,
		rect_positions.size_bytes()
	)));
	staging_buffer.clear();
	}


	//Set draw count
	{
	constexpr sl::uint32_t rect_limit{250};
	//RESULT_VERIFY((sl::universal::get<buffer_id::counts>(inst).try_push_back(
	//	rect_limit
	//)));
	RESULT_VERIFY((staging_buffer.try_push_back(
		rect_limit
	)));
	RESULT_VERIFY(staging_buffer.try_upload_to(sl::universal::get<buffer_id::counts>(inst), sizeof(acma::draw_count_t)));
	//Equivalent to:
	// RESULT_VERIFY(sl::universal::get<buffer_id::counts>(inst).try_resize(sizeof(sl::uint32_t) + sizeof(acma::draw_count_t)));
	// RESULT_VERIFY((acma::copy(
	// 	sl::universal::get<buffer_id::counts>(inst),
	// 	staging_buffer,
	// 	sizeof(decltype(rect_limit)),
	// 	sizeof(acma::draw_count_t)
	// )));
	// staging_buffer.clear();
	}

	//Set offset
	{
	constexpr sl::uint32_t offset{2};
	//std::memcpy(staging_buffer.data(), zeros.data(), staging_buffer.capacity_bytes());
	RESULT_VERIFY((staging_buffer.try_push_back(
		offset
	)));
	RESULT_VERIFY(sl::universal::get<buffer_id::offset>(inst).resize(sizeof(decltype(offset))));
	RESULT_VERIFY((acma::gpu_copy(
		sl::universal::get<buffer_id::offset>(inst),
		staging_buffer,
		sizeof(decltype(offset))
	)));
	staging_buffer.clear();
	}


	//Textures
	llfio::mapped_file_handle texture_mh;
	RESULT_TRY_MOVE(texture_mh, acma::decoder::open_file(assets_path / "test_img_alpha_2.ktx2"));
	RESULT_TRY_MOVE_UNSCOPED(const acma::texture sampled_tex, acma::decoder::decode_texture(texture_mh, acma::texture_usage::sampled), sampled_tex_result);
	llfio::mapped_file_handle font_mh;
	RESULT_TRY_MOVE(font_mh, acma::decoder::open_file(assets_path / "test_font.ktx2"));
	RESULT_TRY_MOVE_UNSCOPED(const acma::texture font_tex, acma::decoder::decode_texture(font_mh, acma::texture_usage::sampled), font_text_result);
	//RESULT_TRY_MOVE_UNSCOPED(const acma::texture storage_t, acma::decoder::decode_texture(texture_mh, acma::texture_usage::storage), storage_tex_result);


	//Sampler
	const VkSamplerCreateInfo sampler_info{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.pNext = nullptr,
		.flags = VkSamplerCreateFlags{},
	    .magFilter               = VK_FILTER_LINEAR,
	    .minFilter               = VK_FILTER_LINEAR,
	    .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
	    .addressModeU            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	    .addressModeV            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
	    .addressModeW            = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
        .mipLodBias              = 0.0f,
        .anisotropyEnable        = VK_TRUE,
        .maxAnisotropy           = inst.physical_device_ptr()->limits.maxSamplerAnisotropy,
        .compareEnable           = VK_FALSE,
        .compareOp               = VK_COMPARE_OP_NEVER,
        .minLod                  = 0.0f,
	    .maxLod                  = static_cast<float>(sampled_tex.mip_level_count),
        .borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
	};
	RESULT_VERIFY(sl::universal::get<asset_heap_id::graphics>(inst).push_back(sl::move(sampler_info)));


	RESULT_VERIFY(sl::universal::get<buffer_id::texture_staging>(inst).push_back(sampled_tex));
	//RESULT_VERIFY(sl::universal::get<buffer_id::texture_staging>(inst).push_back(storage_t));
	RESULT_VERIFY(sl::universal::get<asset_heap_id::graphics>(inst).emplace_back(sl::universal::get<buffer_id::texture_staging>(inst)));

	sl::universal::get<buffer_id::texture_staging>(inst).clear();

	RESULT_VERIFY(sl::universal::get<buffer_id::texture_staging>(inst).push_back(font_tex));
	RESULT_VERIFY(sl::universal::get<asset_heap_id::graphics>(inst).emplace_back(sl::universal::get<buffer_id::texture_staging>(inst)));



	

    //2nd window
    /* {
    auto w2 = app.add_window("sirius Test (Second Window)");
    if(!w.has_value()) return w2.error();
    } */
	 
	{
    //for(auto const& event_fn : acma::input::defaults::interactable::event_fns<window>())
    //    inst.input_event_functions().insert(event_fn);
    //for(auto const& press_binding : acma::input::defaults::interactable::press_bindings<window>())
    //    inst.input_active_bindings().insert(press_binding);
    //for(auto const& release_binding : acma::input::defaults::interactable::release_bindings<window>())
    //    inst.input_inactive_bindings().insert(release_binding);


    acma::input::event_set& left_active = inst.input_active_bindings()[acma::input::combination{{acma::input::generic_code::any}, acma::input::key_code::kb_a}];
    left_active.applicable_categories.set(acma::input::category::system);
    left_active.event_ids[acma::input::category::system] = 0;
    acma::input::event_set& left_inactive = inst.input_inactive_bindings()[acma::input::combination{{acma::input::generic_code::any}, acma::input::key_code::kb_a}];
    left_inactive.applicable_categories.set(acma::input::category::system);
    left_inactive.event_ids[acma::input::category::system] = 1;
    inst.input_event_functions().try_emplace(acma::input::categorized_event_t{0, acma::input::category::system}, [](void*, acma::input::combination, bool pressed, acma::input::categorized_event_t, acma::input::mouse_aux_t, void*){
        std::cout << "left active (by " << (pressed ? std::string_view("press") : std::string_view("release")) << ")" << std::endl;
    });
    inst.input_event_functions().try_emplace(acma::input::categorized_event_t{1, acma::input::category::system}, [](void*, acma::input::combination, bool pressed, acma::input::categorized_event_t, acma::input::mouse_aux_t, void*){
        std::cout << "left inactive (by " << (pressed ? std::string_view("press") : std::string_view("release")) << ")" << std::endl;
    });
    //inst.input_active_bindings().insert_or_assign(acma::input::combination{{}, acma::input::key_code::kb_a}, left_active);
    //inst.input_inactive_bindings().insert_or_assign(acma::input::combination{{acma::input::key_code::kb_a}, acma::input::generic_code::any}, left_active);
    //inst.input_active_bindings().insert_or_assign(acma::input::combination{{acma::input::key_code::kb_a}, acma::input::generic_code::any}, left_inactive);
    //inst.input_inactive_bindings().insert_or_assign(acma::input::combination{{}, acma::input::key_code::kb_a}, left_inactive);
    //inst.input_active_bindings().insert_or_assign(acma::input::combination{{acma::input::generic_code::any}, acma::input::key_code::kb_a}, left_active);
    //inst.input_inactive_bindings().insert_or_assign(acma::input::combination{{acma::input::generic_code::any}, acma::input::key_code::kb_a}, left_inactive);


    acma::input::event_set& ctrl_a_g = inst.input_active_bindings()[acma::input::combination{{acma::input::key_code::kb_left_ctrl, acma::input::key_code::kb_a}, acma::input::key_code::kb_g}];
    ctrl_a_g.applicable_categories.set(acma::input::category::system);
    ctrl_a_g.event_ids[acma::input::category::system] = 2;
    acma::input::event_set& ctrl_g_a = inst.input_active_bindings()[acma::input::combination{{acma::input::key_code::kb_left_ctrl, acma::input::key_code::kb_g}, acma::input::key_code::kb_a}];
    ctrl_g_a.applicable_categories.set(acma::input::category::system);
    ctrl_g_a.event_ids[acma::input::category::system] = 2;
    inst.input_event_functions().try_emplace(acma::input::categorized_event_t{2, acma::input::category::system}, [](void*, acma::input::combination, bool, acma::input::categorized_event_t, acma::input::mouse_aux_t, void*){
        std::cout << "advanced key event called" << std::endl;
    }); 

    inst.current_input_categories().set(acma::input::category::ui);
    //inst.current_input_categories().set(2);

    //acma::input::event_set& inactive_mouse_move = inst.input_inactive_bindings()[acma::input::combination{{acma::input::generic_code::any}, acma::input::mouse_code::move}];
    //inactive_mouse_move.applicable_categories.set(acma::input::category::system);
    //inactive_mouse_move.event_ids[acma::input::category::system] = 3;
    //inst.input_event_functions().try_emplace(acma::input::categorized_event_t{3, acma::input::category::system}, [](void*, acma::input::combination, bool, acma::input::categorized_event_t, acma::input::mouse_aux_t, void*){
    //    std::cout << "mouse move (inactive)" << std::endl;
    //}); 

    acma::input::event_set& shift_mouse_move = inst.input_active_bindings()[acma::input::combination{{acma::input::key_code::kb_left_shift}, acma::input::mouse_code::move}];
    shift_mouse_move.applicable_categories.set(acma::input::category::system);
    shift_mouse_move.event_ids[acma::input::category::system] = 4;
    inst.input_event_functions().try_emplace(acma::input::categorized_event_t{4, acma::input::category::system}, [](void*, acma::input::combination, bool, acma::input::categorized_event_t, acma::input::mouse_aux_t, void*){
        std::cout << "shift mouse move" << std::endl;
    }); 

    acma::input::event_set& shift_scroll_up = inst.input_active_bindings()[acma::input::combination{{acma::input::key_code::kb_left_shift}, acma::input::mouse_code::scroll}];
    shift_scroll_up.applicable_categories.set(acma::input::category::system);
    shift_scroll_up.event_ids[acma::input::category::system] = 5;
    inst.input_event_functions().try_emplace(acma::input::categorized_event_t{5, acma::input::category::system}, [](void*, acma::input::combination, bool, acma::input::categorized_event_t, acma::input::mouse_aux_t scroll_magnitude, void*){
        if(scroll_magnitude->y() >= 0) return;
        std::cout << "shift scroll" << std::endl;
    }); 


    acma::input::event_set& left_mouse_btn = inst.input_active_bindings()[acma::input::combination{{}, acma::input::mouse_code::button_1}];
    left_mouse_btn.applicable_categories.set(acma::input::category::system);
    left_mouse_btn.event_ids[acma::input::category::system] = 6;
    //inst.input_modifier_flags()[acma::input::mouse_code::button_1] |= acma::input::modifier_flags::no_modifiers_allowed;
    inst.input_event_functions().try_emplace(acma::input::categorized_event_t{6, acma::input::category::system}, [](void*, acma::input::combination, bool, acma::input::categorized_event_t, acma::input::mouse_aux_t, void*){
        std::cout << "left mouse button" << std::endl;
    }); 
	}



    std::thread edit_s([](render_instance&) noexcept -> acma::result<void> {//, window& w){
		return {};
    }, std::ref(inst));


    std::future<acma::result<void>> render = inst.start_async_render();

    while(inst.is_open()) {
        inst.poll_events();
        //if(auto r = app.render(); !r.has_value()) [[unlikely]]
        //    return r.error();
    }
    
    if(auto r = render.get(); !r.has_value()) {
        std::cout << std::format("rendering error {}: {}", static_cast<std::int64_t>(r.error()), acma::error::code_descs.find(r.error())->second)<< std::endl;

        auto formats = inst.physical_device_ptr()->query<acma::vk::device_query::display_formats>(*inst.surface_ptr());
        std::cout << std::format("{} formats ({} is selected):", formats.size(), static_cast<std::uint32_t>(inst.swap_chain_ptr()->format().pixel_format.id));
        for(auto f : formats)
            std::cout << std::format("{},", static_cast<std::uint32_t>(f.pixel_format.id));
        std::cout << std::endl;

        auto present_modes = inst.physical_device_ptr()->query<acma::vk::device_query::present_modes>(*inst.surface_ptr());
        std::cout << std::format("{} present_modes ({} is selected):", present_modes.size(), static_cast<std::uint32_t>(inst.swap_chain_ptr()->mode()));
        for(bool b : present_modes)
            std::cout << std::format("{},", b);
        std::cout << std::endl;
    }
    RESULT_VERIFY(inst.join());
    edit_s.join();

    std::cout << acma::error::last_glfw_desc() << std::endl;
    return 0;
}