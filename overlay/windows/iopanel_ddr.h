#pragma once

#include "overlay/window.h"
#include "overlay/windows/iopanel.h"
#include "cfg/button.h"

namespace overlay::windows {

    class DDRIOPanel : public IOPanel {
    public:
        DDRIOPanel(SpiceOverlay *overlay);

    protected:
        void build_io_panel() override;

    private:
        void find_ddr_buttons();
        void draw_buttons(const int player);

        Button *start[2];
        Button *up[2];
        Button *down[2];
        Button *left[2];
        Button *right[2];
        Light *start_light[2];
        Light *updown_light[2];
        Light *leftright_light[2];
    };
}
