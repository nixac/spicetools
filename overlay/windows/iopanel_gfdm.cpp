#include "iopanel_gfdm.h"
#include "avs/game.h"
#include "games/io.h"
#include "games/gitadora/gitadora.h"
#include "games/gitadora/io.h"
#include "misc/eamuse.h"
#include "util/logging.h"

namespace overlay::windows {

    GitadoraIOPanel::GitadoraIOPanel(SpiceOverlay *overlay) : IOPanel(overlay) {
        this->title = "GITADORA IO Panel";

        // by default, make a safer assumption that there are two players
        this->two_players = true;
        // by default, enable the extra input only available on DX cabs...
        this->has_guitar_knobs = true;

        // drummania can only have one player, no guitar knobs
        if (avs::game::is_model({ "J32", "K32", "L32" }) ||
            (avs::game::is_model("M32") && avs::game::SPEC[0] == 'B')) {
            this->two_players = false;
            this->has_guitar_knobs = false;
        }

        // and cab type 3 (white cab) only has one guitar
        if (games::gitadora::CAB_TYPE.has_value() && games::gitadora::CAB_TYPE == 3) {
            this->two_players = false;
        }

        // disable guitar knobs on known non-DX cabs
        if (games::gitadora::CAB_TYPE.has_value() &&
            (games::gitadora::CAB_TYPE == 2 || games::gitadora::CAB_TYPE == 3)) {
            this->has_guitar_knobs = false;
        }

        find_gfdm_buttons();
    }

    void GitadoraIOPanel::find_gfdm_buttons() {
        const auto buttons = games::get_buttons(eamuse_get_game());

        // device emulation treats drum controls to be the same as guitar 1p

        this->start[0] = &(*buttons)[games::gitadora::Buttons::GuitarP1Start];
        this->help[0] = &(*buttons)[games::gitadora::Buttons::GuitarP1Help];
        this->up[0] = &(*buttons)[games::gitadora::Buttons::GuitarP1Up];
        this->down[0] = &(*buttons)[games::gitadora::Buttons::GuitarP1Down];
        this->left[0] = &(*buttons)[games::gitadora::Buttons::GuitarP1Left];
        this->right[0] = &(*buttons)[games::gitadora::Buttons::GuitarP1Right];

        this->start[1] = &(*buttons)[games::gitadora::Buttons::GuitarP2Start];
        this->help[1] = &(*buttons)[games::gitadora::Buttons::GuitarP2Help];
        this->up[1] = &(*buttons)[games::gitadora::Buttons::GuitarP2Up];
        this->down[1] = &(*buttons)[games::gitadora::Buttons::GuitarP2Down];
        this->left[1] = &(*buttons)[games::gitadora::Buttons::GuitarP2Left];
        this->right[1] = &(*buttons)[games::gitadora::Buttons::GuitarP2Right];
    }

    void GitadoraIOPanel::build_io_panel() {
        ImGui::Dummy(ImVec2(12, 0));

        ImGui::SameLine();
        this->draw_buttons(0);
        if (this->has_guitar_knobs) {
            ImGui::SameLine();
            this->draw_sliders(0);
        }

        // draw p2 only if guitar freaks
        if (this->two_players) {
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(12, 0));
            ImGui::SameLine();
            this->draw_buttons(1);
            if (this->has_guitar_knobs) {
                ImGui::SameLine();
                this->draw_sliders(1);
            }
        }
    }

    void GitadoraIOPanel::draw_buttons(const int p) {
        // 2x2
        const ImVec2 start_button_size(
            ImGui::GetFrameHeightWithSpacing() + ImGui::GetFrameHeight(),
            ImGui::GetFrameHeightWithSpacing() + ImGui::GetFrameHeight()
            );
        // 2x1
        const ImVec2 updown_size(start_button_size.x, ImGui::GetFrameHeight());
        // 1x2
        const ImVec2 leftright_size(ImGui::GetFrameHeight(), start_button_size.y);
        // 1x1
        const ImVec2 tiny_size(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());

        ImGui::BeginGroup();
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetFrameHeightWithSpacing());
            this->build_button("<", leftright_size, this->left[p]);
        }
        ImGui::EndGroup();

        ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.y);

        ImGui::BeginGroup();
        {
            this->build_button("^", updown_size, this->up[p]);
            const char *label;
            if (this->two_players) {
                if (p == 0) {
                    label = " 1 P\nStart";
                } else {
                    label = " 2 P\nStart";
                }
            } else {
                label = "Start";
            }
            this->build_button(label, start_button_size, this->start[p]);
            this->build_button("v", updown_size, this->down[p]);
        }
        ImGui::EndGroup();

        ImGui::SameLine(0, ImGui::GetStyle().ItemSpacing.y);

        ImGui::BeginGroup();
        {
            this->build_button("?", tiny_size, this->help[p]);
            this->build_button(">", leftright_size, this->right[p]);
        }
        ImGui::EndGroup();
    }

    void GitadoraIOPanel::draw_sliders(const int p) {
        const auto index =
            (p == 0) ?
            games::gitadora::Analogs::GuitarP1Knob :
            games::gitadora::Analogs::GuitarP2Knob;

        const ImVec2 slider_size(32, get_suggested_height());

        // get analog
        const auto analogs = games::get_analogs(eamuse_get_game());
        auto &analog = (*analogs)[index];

        // vertical slider
        auto new_state = analog.override_enabled ? analog.override_state
                : GameAPI::Analogs::getState(RI_MGR, analog);

        ImGui::PushID(index);
        ImGui::VSliderFloat(
            "##v",
            slider_size,
            &new_state,
            0.f, 1.f,
            "Knob",
            ImGuiSliderFlags_AlwaysClamp);
        ImGui::PopID();
        if (new_state != analog.override_state) {
            analog.override_state = new_state;
            analog.override_enabled = true;
        }
    }
}
