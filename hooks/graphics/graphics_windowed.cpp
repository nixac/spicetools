#include "graphics.h"

#include "avs/game.h"
#include "cfg/screen_resize.h"
#include "overlay/overlay.h"
#include "util/logging.h"
#include "util/utils.h"

#if 0
#define log_debug(module, format_str, ...) logger::push( \
    LOG_FORMAT("M", module, format_str, ## __VA_ARGS__), logger::Style::GREY)
#else
#define log_debug(module, format_str, ...)
#endif

void graphics_load_windowed_parameters();
void graphics_wm_style_changed(HWND hWnd, bool changed);
void graphics_wm_sizing_aspect_ratio(int edge, RECT& rect);

std::optional<int> GRAPHICS_WINDOW_STYLE;
std::optional<std::string> GRAPHICS_WINDOW_SIZE;
std::optional<std::string> GRAPHICS_WINDOW_POS;
bool GRAPHICS_WINDOW_ALWAYS_ON_TOP = false;

// these flags are carefully constructed to ensure maximum compatibility
// (e.g., DDR likes to hang when SetWindowPos is called with certain params)
static const DWORD SETWINDOWPOS_NOOP = 
    SWP_NOMOVE |
    SWP_NOSIZE |
    SWP_NOREDRAW |
    SWP_NOCOPYBITS |
    SWP_NOACTIVATE |
    SWP_NOSENDCHANGING |
    SWP_DEFERERASE |
    SWP_NOZORDER |
    SWP_ASYNCWINDOWPOS;

void graphics_capture_initial_window(HWND hWnd) {
    if (!GRAPHICS_WINDOWED) {
        return;
    }

    graphics_load_windowed_parameters();

    cfg::SCREENRESIZE->init_window_style = GetWindowLong(hWnd, GWL_STYLE);

    log_debug("graphics-windowed", "graphics_capture_initial_window called");

    RECT rect;
    if (!GetClientRect(hWnd, &rect)) {
        log_warning(
            "graphics",
            "graphics_capture_initial_window - GetClientRect failed, GLE: {}",
            GetLastError());
        return;
    }
    const int client_w = rect.right - rect.left;
    const int client_h = rect.bottom - rect.top;

    cfg::SCREENRESIZE->init_client_width = client_w;
    cfg::SCREENRESIZE->init_client_height = client_h;
    cfg::SCREENRESIZE->init_client_aspect_ratio = (float)client_w / (float)client_h;
    log_debug(
        "graphics-windowed",
        "graphics_capture_initial_window initial window size {}x{}, ratio {}",
        client_w, client_h, cfg::SCREENRESIZE->init_client_aspect_ratio);

    // ensure frame size is captured
    graphics_wm_style_changed(hWnd, false);

    // if there was no user-supplied dimension, seed it with the current size
    // so that the next resize operation will work
    if (cfg::SCREENRESIZE->client_width == 0) {
        cfg::SCREENRESIZE->client_width = client_w;
    }
    if (cfg::SCREENRESIZE->client_height == 0) {
        cfg::SCREENRESIZE->client_height = client_h;
    }

    // apply the config that was loaded from disk
    // resize must be done before applying the border
    if (cfg::SCREENRESIZE->enable_window_resize) {
        log_info(
            "graphics-windowed", "change window rect - window offset: {}x{}, client size: {}x{}",
            cfg::SCREENRESIZE->window_offset_x,
            cfg::SCREENRESIZE->window_offset_y,
            cfg::SCREENRESIZE->client_width,
            cfg::SCREENRESIZE->client_height);
        graphics_move_resize_window(hWnd);
    }
    // ddr hangs when window frame doesn't have overlapped
    if (cfg::SCREENRESIZE->window_decoration != cfg::WindowDecorationMode::Default) {
        log_info(
            "graphics-windowed", "change window style - decoration: {}",
            cfg::SCREENRESIZE->window_decoration);
        graphics_update_window_style(hWnd);
    }
    if (cfg::SCREENRESIZE->window_always_on_top) {
        log_info("graphics-windowed", "change window z-order - always on top");
        graphics_update_z_order(hWnd);
    }

    log_debug("graphics-windowed", "graphics_capture_initial_window returned");
}

void graphics_load_windowed_parameters() {
    if (!GRAPHICS_WINDOWED) {
        return;
    }

    log_debug("graphics-windowed", "graphics_load_windowed_parameters called");
    const auto remove_spaces = [](const char& c) {
        return c == ' ';
    };

    if (GRAPHICS_WINDOW_STYLE.has_value()) {
        log_debug(
            "graphics-windowed",
            "graphics_load_windowed_parameters - load GRAPHICS_WINDOW_STYLE");
        cfg::SCREENRESIZE->window_decoration = GRAPHICS_WINDOW_STYLE.value();
    }

    if (GRAPHICS_WINDOW_SIZE.has_value()) {
        log_debug(
            "graphics-windowed",
            "graphics_load_windowed_parameters - load GRAPHICS_WINDOW_SIZE");
        uint32_t w, h;
        auto s = GRAPHICS_WINDOW_SIZE.value();
        s.erase(std::remove_if(s.begin(), s.end(), remove_spaces), s.end());
        if (sscanf(s.c_str(), "%u,%u", &w, &h) == 2) {
            cfg::SCREENRESIZE->enable_window_resize = true;
            cfg::SCREENRESIZE->client_keep_aspect_ratio = false;
            cfg::SCREENRESIZE->client_width = w;
            cfg::SCREENRESIZE->client_height = h;
        } else {
            log_warning("graphics-windowed", "failed to parse -sp2x-windowsize");
        }
    }

    if (GRAPHICS_WINDOW_POS.has_value()) {
        log_debug(
            "graphics-windowed",
            "graphics_load_windowed_parameters - load GRAPHICS_WINDOW_POS");
        int32_t x, y;
        auto s = GRAPHICS_WINDOW_POS.value();
        s.erase(std::remove_if(s.begin(), s.end(), remove_spaces), s.end());
        if (sscanf(s.c_str(), "%d,%d", &x, &y) == 2) {
            cfg::SCREENRESIZE->enable_window_resize = true;
            cfg::SCREENRESIZE->window_offset_x = x;
            cfg::SCREENRESIZE->window_offset_y = y;
        } else {
            log_warning("graphics-windowed", "failed to parse -sp2x-windowpos");
        }
    }

    // only override if true; don't stomp on user setting
    if (GRAPHICS_WINDOW_ALWAYS_ON_TOP) {
        cfg::SCREENRESIZE->window_always_on_top = true;
    }

    log_debug("graphics-windowed", "graphics_load_windowed_parameters returned");
}

void graphics_windowed_wndproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!GRAPHICS_WINDOWED) {
        return;
    }

    switch (uMsg) {
        case WM_MOVE: {
            log_debug("graphics-windowed", "graphics_windowed_wndproc called with WM_MOVE");
            RECT rect;
            if (GetWindowRect(hWnd, &rect)) {
                cfg::SCREENRESIZE->window_offset_x = rect.left;
                cfg::SCREENRESIZE->window_offset_y = rect.top;
            }
            break;
        }
        case WM_SIZE: {
            log_debug("graphics-windowed", "graphics_windowed_wndproc called with WM_SIZE");
            if (wParam == SIZE_MINIMIZED) {
                break;
            }
            RECT rect;
            if (GetClientRect(hWnd, &rect)) {
                cfg::SCREENRESIZE->client_width = rect.right - rect.left;
                cfg::SCREENRESIZE->client_height = rect.bottom - rect.top;
            }
            break;
        }
        case WM_SIZING: {
            if (cfg::SCREENRESIZE->client_keep_aspect_ratio) {
                graphics_wm_sizing_aspect_ratio(
                    static_cast<int>(wParam), *reinterpret_cast<LPRECT>(lParam));
            }
            break;
        }
        case WM_STYLECHANGED: {
            graphics_wm_style_changed(hWnd, true);
            break;
        }
        case WM_GETMINMAXINFO: {
            if (cfg::SCREENRESIZE->client_keep_aspect_ratio) {
                auto info = reinterpret_cast<MINMAXINFO *>(lParam);
                info->ptMinTrackSize.y =
                    cfg::SCREENRESIZE->window_deco_height +
                    ((info->ptMinTrackSize.x - cfg::SCREENRESIZE->window_deco_width) /
                        cfg::SCREENRESIZE->init_client_aspect_ratio);
            }
            break;
        }
    }
}

void graphics_wm_style_changed(HWND hWnd, bool changed) {
    log_debug("graphics-windowed", "graphics_wm_style_changed called");
    RECT rect;

    // ensure the style change takes in effect before doing the calculations
    if (changed) {
        // ensure client size doesn't change as a result of this
        // since SetWindowPos will still send WM_SIZE and WM_MOVE
        const auto client_w = cfg::SCREENRESIZE->client_width;
        const auto client_h = cfg::SCREENRESIZE->client_height;
        const auto flags = SETWINDOWPOS_NOOP | SWP_FRAMECHANGED;
        SetWindowPos(
            hWnd,
            nullptr,
            0, 0, 0, 0,
            flags);
        cfg::SCREENRESIZE->client_width = client_w;
        cfg::SCREENRESIZE->client_height = client_h;
    }

    // get window size with decoration...
    if (!GetWindowRect(hWnd, &rect)) {
        log_warning(
            "graphics",
            "graphics_wm_style_changed - GetWindowRect failed, GLE: {}",
            GetLastError());
        return;
    }
    const int window_w = rect.right - rect.left;
    const int window_h = rect.bottom - rect.top;

    // get client area (without decoration)...
    if (!GetClientRect(hWnd, &rect)) {
        return;
    }
    const int client_w = rect.right - rect.left;
    const int client_h = rect.bottom - rect.top;

    // update window decoration size
    cfg::SCREENRESIZE->window_deco_width = window_w - client_w;
    cfg::SCREENRESIZE->window_deco_height = window_h - client_h;
    log_debug(
        "graphics-windowed",
        "graphics_wm_style_changed updating frame dimensions {}x{}",
        cfg::SCREENRESIZE->window_deco_width,
        cfg::SCREENRESIZE->window_deco_height);

    // adjust window to ensure client area remains the same
    if (changed) {
        graphics_move_resize_window(hWnd);
    }

    log_debug("graphics-windowed", "graphics_wm_style_changed returned");
}

void graphics_wm_sizing_aspect_ratio(int edge, RECT& rect) {
    log_debug("graphics-windowed", "graphics_wm_sizing_aspect_ratio called");

    const auto deco_w = cfg::SCREENRESIZE->window_deco_width;
    const auto deco_h = cfg::SCREENRESIZE->window_deco_height;
    const LONG desired_w = (rect.right - rect.left) - deco_w;
    const LONG desired_h = (rect.bottom - rect.top) - deco_h;
    const auto aspect_ratio = cfg::SCREENRESIZE->init_client_aspect_ratio;

    // based on http://playtechs.blogspot.com/2007/10/forcing-window-to-maintain-particular.html
    switch (edge) {
        case WMSZ_BOTTOM:
        case WMSZ_TOP: {
            const LONG w = deco_w + (desired_h * aspect_ratio);
            rect.right = rect.left + w;
            break;
        }
        case WMSZ_LEFT:
        case WMSZ_RIGHT: {
            const LONG h = deco_h + (desired_w / aspect_ratio);
            rect.bottom = rect.top + h;
            break;
        }
        case WMSZ_TOPLEFT:
        case WMSZ_TOPRIGHT:
        case WMSZ_BOTTOMLEFT:
        case WMSZ_BOTTOMRIGHT: {
            int w;
            int h;
            if (desired_h * aspect_ratio < desired_w) {
                w = rect.right - rect.left;
                h = deco_h + (desired_w / aspect_ratio);
            } else {
                w = deco_w + (desired_h * aspect_ratio);
                h = rect.bottom - rect.top;
            }
            if (edge == WMSZ_TOPLEFT) {
                rect.left = rect.right - w;
                rect.top = rect.bottom - h;
            } else if (edge == WMSZ_TOPRIGHT) {
                rect.right = rect.left + w;
                rect.top = rect.bottom - h;
            } else if (edge == WMSZ_BOTTOMLEFT) {
                rect.left = rect.right - w;
                rect.bottom = rect.top + h;
            } else if (edge == WMSZ_BOTTOMRIGHT) {
                rect.right = rect.left + w;
                rect.bottom = rect.top + h;
            }
            break;
        }
        default:
            break;
    }

    log_debug("graphics-windowed", "graphics_wm_sizing_aspect_ratio returned");
}

void graphics_update_window_style(HWND hWnd) {
    if (!GRAPHICS_WINDOWED) {
        return;
    }
    if (graphics_window_change_crashes_game()) {
        return;
    }

    log_debug("graphics-windowed", "graphics_update_window_style called");

    // update frame style
    auto style = cfg::SCREENRESIZE->init_window_style;
    switch (cfg::SCREENRESIZE->window_decoration) {
        case cfg::WindowDecorationMode::Borderless:
            style &= ~WS_OVERLAPPEDWINDOW;
            break;
        case cfg::WindowDecorationMode::ResizableFrame:
            style |= WS_OVERLAPPEDWINDOW;
            break;
        case cfg::WindowDecorationMode::Default:
        default:
            break;
    }

    log_debug(
        "graphics-windowed",
        "graphics_update_window_style - calling SetWindowLong with Mode {}, style 0x{:x}",
        static_cast<int>(cfg::SCREENRESIZE->window_decoration),
        style);
    SetWindowLong(hWnd, GWL_STYLE, style);

    // SetWindowPos must be called after SetWindowLong if the frame style changed
    // this will be done in WM_STYLECHANGED handler
    log_debug("graphics-windowed", "graphics_update_window_style returned");
}

void graphics_update_z_order(HWND hWnd) {
    if (!GRAPHICS_WINDOWED) {
        return;
    }

    log_debug("graphics-windowed", "graphics_update_z_order called");

    HWND insert_after = nullptr;
    if (cfg::SCREENRESIZE->window_always_on_top) {
        insert_after = HWND_TOPMOST;
    } else {
        insert_after = HWND_NOTOPMOST;
    }

    auto flags = SETWINDOWPOS_NOOP;
    flags &= ~SWP_NOZORDER;
    SetWindowPos(
        hWnd,
        insert_after,
        0, 0, 0, 0,
        flags);
    
    log_debug("graphics-windowed", "graphics_update_z_order returned");
}

void graphics_move_resize_window(HWND hWnd) {
    if (!GRAPHICS_WINDOWED) {
        return;
    }

    log_debug("graphics-windowed", "graphics_move_resize_window called");

    cfg::SCREENRESIZE->client_width =
        CLAMP(cfg::SCREENRESIZE->client_width, 640, 1920 * 8);
    cfg::SCREENRESIZE->client_height =
        CLAMP(cfg::SCREENRESIZE->client_height, 480, 1080 * 8);

    const auto w = cfg::SCREENRESIZE->client_width + cfg::SCREENRESIZE->window_deco_width;
    const auto h = cfg::SCREENRESIZE->client_height + cfg::SCREENRESIZE->window_deco_height;

    auto flags = SETWINDOWPOS_NOOP;
    flags &= ~SWP_NOSIZE;
    flags &= ~SWP_NOMOVE;
    SetWindowPos(
        hWnd,
        nullptr,
        cfg::SCREENRESIZE->window_offset_x,
        cfg::SCREENRESIZE->window_offset_y,
        w, h,
        flags);

    log_debug("graphics-windowed", "graphics_move_resize_window returned");
}

bool graphics_window_change_crashes_game() {
    static std::once_flag flag;
    static bool result = false;
    std::call_once(flag, []() {
        // ddr crashes when frame style changes
        result = avs::game::is_model("MDX");
        if (result) {
            log_warning(
                "graphics-windowed",
                "ignoring changes to window style due to incompatibility with this game");
        }
    });
    return result;
}
