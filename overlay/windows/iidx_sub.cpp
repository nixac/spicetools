#undef CINTERFACE

#include "iidx_sub.h"
#include "games/iidx/iidx.h"

namespace overlay::windows {

    IIDXSubScreen::IIDXSubScreen(SpiceOverlay *overlay) : GenericSubScreen(overlay) {
        this->title = "IIDX Sub Screen";

        float size = 0.5f;
        if (games::iidx::SUBSCREEN_OVERLAY_SIZE.has_value()) {
            if (games::iidx::SUBSCREEN_OVERLAY_SIZE.value() == "large") {
                size = 0.8f;
            } else if (games::iidx::SUBSCREEN_OVERLAY_SIZE.value() == "small") {
                size = 0.3f;
            } else if (games::iidx::SUBSCREEN_OVERLAY_SIZE.value() == "fullscreen") {
                this->draws_window = false;
            }
        }
        this->init_size = ImVec2(
            ImGui::GetIO().DisplaySize.x * size,
            (ImGui::GetIO().DisplaySize.x * size * 9 / 16) + ImGui::GetFrameHeight());

        this->size_max = ImVec2(
            ImGui::GetIO().DisplaySize.x - ImGui::GetFrameHeight() * 2,
            ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight() * 2);

        // middle / bottom
        this->init_pos = ImVec2(
            ImGui::GetIO().DisplaySize.x / 2 - this->init_size.x / 2,
            ImGui::GetIO().DisplaySize.y - this->init_size.y - (ImGui::GetFrameHeight() / 2));
    }

    void IIDXSubScreen::touch_transform(const ImVec2 xy_in, LONG *x_out, LONG *y_out) {
        if (!this->get_active()) {
            return;
        }

        *x_out = xy_in.x * ImGui::GetIO().DisplaySize.x;
        *y_out = xy_in.y * ImGui::GetIO().DisplaySize.y;
    }
}
