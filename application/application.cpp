#include "application.hpp"
#include <stdexcept>
#include "sprite.hpp"

using namespace std;
using namespace Vulkan;

namespace Granite
{
Application::Application(unsigned width, unsigned height)
{
	EventManager::get_global();
	Filesystem::get();

	platform = create_default_application_platform(width, height);

	if (!wsi.init(platform.get(), width, height))
		throw runtime_error("Failed to initialize WSI.");
}

SceneViewerApplication::SceneViewerApplication(const std::string &path, unsigned width, unsigned height)
	: Application(width, height)
{
	scene_loader.load_scene(path);
	animation_system = scene_loader.consume_animation_system();

	cam.look_at(vec3(0.0f, 0.0f, 8.0f), vec3(0.0f));
	context.set_camera(cam);
	font.reset(new Font("assets://font.ttf", 12));
}

void SceneViewerApplication::render_frame(double, double elapsed_time)
{
	auto &wsi = get_wsi();
	auto &scene = scene_loader.get_scene();
	auto &device = wsi.get_device();

	animation_system->animate(elapsed_time);
	context.set_camera(cam);
	visible.clear();

	flat_renderer.begin();

	Camera camera;
	camera.look_at(vec3(0.0f), vec3(1.0f, 0.0f, 0.0f));
	camera.set_depth_range(0.1f, 3.0f);
	mat4 inv_vp = inverse(camera.get_projection() * camera.get_view());
	Frustum f;
	f.build_planes(inv_vp);

	scene.update_cached_transforms();
	scene.gather_visible_opaque_renderables(context.get_visibility_frustum(), visible);
	scene.gather_background_renderables(visible);

	auto cmd = device.request_command_buffer();
	auto rp = device.get_swapchain_render_pass(SwapchainRenderPass::DepthStencil);
	cmd->begin_render_pass(rp);
	renderer.begin();
	renderer.push_renderables(context, visible);
	renderer.render_debug_frustum(context, f, vec4(0.0f, 0.0f, 1.0f, 1.0f));
	renderer.flush(*cmd, context);
	flat_renderer.flush(*cmd, vec2(0.0f), vec2(cmd->get_viewport().width, cmd->get_viewport().height));
	cmd->end_render_pass();
	device.submit(cmd);
}

int Application::run()
{
	auto &wsi = get_wsi();
	while (get_platform().alive(wsi))
	{
		Filesystem::get().poll_notifications();
		wsi.begin_frame();
		render_frame(wsi.get_platform().get_frame_timer().get_frame_time(),
					 wsi.get_platform().get_frame_timer().get_elapsed());
		wsi.end_frame();
	}
	return 0;
}

}
