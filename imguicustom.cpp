#include "imgui.h"
#include "imgui_internal.h"
#include "imguiCustom.h"

void ImGuiCustom::colorPicker(const char* name, float color[3], bool* enable) noexcept
{
	ImGui::PushID(name);
	if (enable) {
		ImGui::Checkbox("##check", enable);
		ImGui::SameLine(0.0f, 5.0f);
	}
	bool openPopup = ImGui::ColorButton("##btn", color, ImGuiColorEditFlags_NoTooltip);
	ImGui::SameLine(0.0f, 5.0f);
	ImGui::TextUnformatted(name);

	if (openPopup)
		ImGui::OpenPopup("##popup");

	if (ImGui::BeginPopup("##popup")) {
		ImGui::ColorPicker3("##picker", color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview);
		ImGui::EndPopup();
	}
	ImGui::PopID();
}


