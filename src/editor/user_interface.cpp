#include "user_interface.h"

#include <fstream>
#include <functional>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>
#include <memory>
#include <SDL_events.h>
#include <string>

#include "../math/math_utility.h"
#include "../render/camera.h"
#include "../render/image.h"
#include "../scenes/scenes.h"
#include "window.h"

UserInterface::UserInterface(Window& window, std::function<std::string()> save_func) : _window(window), _save_func(save_func) {
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	_io = &ImGui::GetIO();
	_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplSDL2_InitForSDLRenderer(_window.get_sdl_window(), _window.get_sdl_renderer());
	ImGui_ImplSDLRenderer2_Init(_window.get_sdl_renderer());
}

UserInterface::~UserInterface() {
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void UserInterface::process_event(const SDL_Event* e) {
	ImGui_ImplSDL2_ProcessEvent(e);
}

void UserInterface::update() {
	if (_camera == nullptr) return;

	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	{
		ImGui::SetNextWindowPos(ImVec2{ 0.f, 0.f });
		ImGui::SetNextWindowSize(ImVec2{ static_cast<float>(width), static_cast<float>(_window.get_height()) });
		ImGui::Begin("Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		// ACTIONS
		if (ImGui::Button("Render")) {
			_camera->render();
			_save_name = "";
		}
		ImGui::SameLine();
		if (ImGui::Button("Quick render")) {
			_camera->render(true);
			_save_name = "";
		}

		if (_camera->is_rendering()) {
			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				_camera->stop_render();
			}
			else {
				ImGui::SameLine();
				auto text = "Rendering..." + std::to_string(_camera->get_n_pixel_renders_remaining());
				ImGui::Text(text.c_str());
			}
		}
		else if (_camera->get_render()) {
			if (ImGui::Button("Save")) {
				_save_name = _save_func();
			}
			if (_save_name != "") {
				ImGui::SameLine();
				auto text = "Saved as " + _save_name;
				ImGui::Text(text.c_str());
			}
		}

		// SCENE SETUP
		if (ImGui::CollapsingHeader("Scene setup", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::Combo("Scene", &_current_scene_selection, Scenes::scene_names, IM_ARRAYSIZE(Scenes::scene_names))) {
				update_scene();
			}

			ImGui::ColorEdit3("Background color 1", _camera->background_primary.components, ImGuiColorEditFlags_NoInputs);
			ImGui::ColorEdit3("Background color 2", _camera->background_secondary.components, ImGuiColorEditFlags_NoInputs);
		}

		// RENDER QUALITY
		if (ImGui::CollapsingHeader("Render quality", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderInt("Image width", &_camera->image_width, 1, 3840);
			ImGui::SliderInt("Samples per pixel", &_camera->high_quality_render_samples, 1, 1000);
			ImGui::SliderInt("Max bounces", &_camera->max_bounces, 1, 100);
		}

		// CAMERA SETTINGS
		if (ImGui::CollapsingHeader("Camera settings", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::SliderFloat("Vertical FOV", &_camera->vfov, 1.f, 179.f);
			ImGui::SliderFloat("Defocus strength", &_camera->defocus_angle, 0.f, 10.f);
			ImGui::SliderFloat("Focus distance", &_camera->focus_distance, 0.01f, 100.f);
			ImGui::InputFloat3("Position", _camera->pos.components);
			ImGui::InputFloat3("Look at", _camera->look_at.components);
		}

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
}

void UserInterface::set_camera(std::shared_ptr<Camera> camera) {
	_camera = camera;
}

void UserInterface::update_scene() {
	if (_camera == nullptr) return;
	_camera->set_scene(Scenes::instance_scene(_current_scene_selection));
}
