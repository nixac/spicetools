#include "options.h"

#include "external/tinyxml2/tinyxml2.h"
#include "util/utils.h"
#include "util/fileutils.h"

#include <fstream>

static const std::vector<std::string> CATEGORY_ORDER_BASIC = {
    "Game Options",
    "Common",
    "Overlay",
    "Graphics (Common)",
    "Graphics (Windowed)",
    "Audio",
    "SpiceCompanion and API",
};

static const std::vector<std::string> CATEGORY_ORDER_ADVANCED = {
    "Game Options (Advanced)",
    "Network",
    "Performance",
    "Miscellaneous",
    "Paths",
    "Touch Parameters",
    "I/O Modules",
    "Card Readers",
    "VR",
    "Development",
    "SpiceCompanion and API (Advanced)",
};

/*
 * Option Definitions
 * Be aware that the order must be the same as in the enum launcher::Options!
 */
static const std::vector<OptionDefinition> OPTION_DEFINITIONS = {
    {
        .title = "Game Executable",
        .name = "exec",
        .desc = "Path to the game DLL file",
        .type = OptionType::Text,
        .setting_name = "*.dll",
    },
    {
        .title = "Open Configurator",
        .name = "cfg",
        .desc = "Open configuration window",
        .type = OptionType::Bool,
        .hidden = true,
    },
    {
        .title = "Open KFControl",
        .name = "kfcontrol",
        .desc = "Open configuration window",
        .type = OptionType::Bool,
        .hidden = true,
    },
    {
        .title = "Basic Local EA Emulation",
        .name = "ea",
        .desc = "Enables the integrated local EA server, just enough to boot the game; no card in, no data saving",
        .type = OptionType::Bool,
        .category = "Common",
    },
    {
        .title = "EA Service URL",
        .name = "url",
        .desc = "Sets a custom service URL override",
        .type = OptionType::Text,
        .category = "Common",
    },
    {
        .title = "PCBID",
        .name = "p",
        .desc = "Sets a custom PCBID override",
        .type = OptionType::Text,
        .setting_name = "04040000000000000000",
        .category = "Common",
        .sensitive = true,
    },
    {
        .title = "Player 1 Card",
        .name = "card0",
        .desc = "Set a card number for reader 1. Overrides the selected card file.",
        .type = OptionType::Text,
        .setting_name = "E004000000000000",
        .category = "Common",
        .sensitive = true,
    },
    {
        .title = "Player 2 Card",
        .name = "card1",
        .desc = "Set a card number for reader 2. Overrides the selected card file.",
        .type = OptionType::Text,
        .setting_name = "E004000000000000",
        .category = "Common",
        .sensitive = true,
    },
    {
        .title = "Windowed Mode",
        .name = "w",
        .desc = "Enables windowed mode",
        .type = OptionType::Bool,
        .category = "Graphics (Windowed)",
    },
    {
        .title = "Inject DLL Hook",
        .name = "k",
        .desc = "Multiple files are allowed; use multiple -k flags, or in SpiceCfg, separate by "
                "space (foo.dll bar.dll baz.dll). Injects a hook by using LoadLibrary on the "
                "specified file before running the main game code",
        .type = OptionType::Text,
        .setting_name = "a.dll b.dll c.dll",
        .category = "Common",
    },
    {
        .title = "Execute Script",
        .name = "script",
        .desc = "Executes a script (.lua) at the given path on game boot",
        .type = OptionType::Text,
        .category = "Miscellaneous",
    },
    {
        .title = "Lock Cursor to Window",
        .name = "c",
        .desc = "Confines the cursor to be within the game window",
        .type = OptionType::Bool,
        .category = "Common",
    },
    {
        .title = "Always Show Cursor",
        .name = "s",
        .desc = "Shows the cursor in the game window, useful for games with touch support",
        .type = OptionType::Bool,
        .category = "Common",
    },
    {
        .title = "Monitor",
        .name = "monitor",
        .desc = "Sets the display that the game will be opened in, for multiple monitors",
        .type = OptionType::Integer,
        .category = "Graphics (Common)",
    },
    {
        .title = "Force Refresh Rate",
        .name = "graphics-force-refresh",
        .desc = "Force the refresh rate for the primary graphics window",
        .type = OptionType::Integer,
        .category = "Graphics (Common)",
    },
    {
        .title = "Only Use One Monitor",
        .name = "graphics-force-single-adapter",
        .desc = "Force the graphics device to be opened utilizing only one adapter in multi-monitor systems",
        .type = OptionType::Bool,
        .category = "Graphics (Common)",
    },
    {
        .title = "DirectX 9 on 12",
        .name = "9on12",
        .desc = "Use D3D9On12 wrapper library, requires Windows 10 Insider Preview 18956 or later",
        .type = OptionType::Bool,
        .category = "Graphics (Common)",
    },
    {
        .title = "Disable Win/Media/Special Keys",
        .name = "nolegacy",
        .desc = "Disables legacy key activation in-game",
        .type = OptionType::Bool,
        .category = "Miscellaneous",
    },
    {
        .title = "Discord Rich Presence",
        .name = "richpresence",
        .desc = "Enables Discord Rich Presence support",
        .type = OptionType::Bool,
        .category = "Miscellaneous",
    },
    {
        .title = "Smart EA",
        .name = "smartea",
        .desc = "Automatically enables -ea when server is offline",
        .type = OptionType::Bool,
        .category = "Network",
    },
    {
        .title = "EA Maintenance",
        .name = "eamaint",
        .desc = "Enables EA Maintenance, 1 for on, 0 for off",
        .type = OptionType::Enum,
        .category = "Network",
        .elements = {{"0", "Off"}, {"1", "On"}},
    },
    {
        .title = "Adapter Network",
        .name = "network",
        .desc = "Force the use of an adapter with the specified network",
        .type = OptionType::Text,
        .category = "Network",
    },
    {
        .title = "Adapter Subnet",
        .name = "subnet",
        .desc = "Force the use of an adapter with the specified subnet",
        .type = OptionType::Text,
        .category = "Network",
    },
    {
        .title = "Disable Network Fixes",
        .name = "netfixdisable",
        .desc = "Force disables network fixes",
        .type = OptionType::Bool,
        .category = "Network",
    },
    {
        .title = "HTTP/1.1",
        .name = "http11",
        .desc = "Sets EA3 http11 value",
        .type = OptionType::Enum,
        .category = "Network",
        .elements = {{"0", "Off"}, {"1", "On"}},
    },
    {
        .title = "Disable SSL Protocol",
        .name = "ssldisable",
        .desc = "Prevents the SSL protocol from being registered",
        .type = OptionType::Bool,
        .category = "Network",
    },
    {
        .title = "URL Slash",
        .name = "urlslash",
        .desc = "Sets EA3 urlslash value",
        .type = OptionType::Enum,
        .category = "Network",
        .elements = {{"0", "Off"}, {"1", "On"}},
    },
    {
        .title = "SOFTID",
        .name = "r",
        .desc = "Set custom SOFTID override",
        .type = OptionType::Text,
        .category = "Network",
        .sensitive = true,
    },
    {
        .title = "Enable VR",
        .name = "vr",
        .desc = "Enable VR support",
        .type = OptionType::Bool,
        .category = "VR",
    },
    {
        .title = "Disable Spice Overlay",
        .name = "overlaydisable",
        .desc = "Disables the in-game overlay",
        .type = OptionType::Bool,
        .category = "Overlay",
    },
    {
        // spice2x_FpsAutoShow
        .title = "Auto Show FPS/Clock",
        .name = "sp2x-autofps",
        .desc = "Automatically show FPS / clock / timer overlay window when the game starts",
        .type = OptionType::Bool,
        .category = "Overlay",
    },
    {
        // spice2x_SubScreenAutoShow
        .title = "Auto Show Subscreen",
        .name = "sp2x-autosubscreen",
        .desc = "Automatically show the subscreen overlay when the game starts, if the game has one",
        .type = OptionType::Bool,
        .category = "Overlay",
    },
    {
        // spice2x_IOPanelAutoShow
        .title = "Auto Show IO Panel",
        .name = "sp2x-autoiopanel",
        .desc = "Automatically show I/O panel window when the game starts",
        .type = OptionType::Bool,
        .category = "Overlay",
    },
    {
        // spice2x_KeypadAutoShow
        .title = "Auto Show Keypad",
        .name = "sp2x-autokeypad",
        .desc = "Automatically show virtual keypad window when the game starts",
        .type = OptionType::Enum,
        .category = "Overlay",
        .elements = {
            {"0", "Off"},
            {"1", "P1"},
            {"2", "P2"},
            {"3", "P1 and P2"},
        },
    },
    {
        .title = "Force Load IIDX Module",
        .name = "iidx",
        .desc = "Manually enable Beatmania IIDX module",
        .type = OptionType::Bool,
        .game_name = "Beatmania IIDX",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "IIDX Camera Order Flip",
        .name = "iidxflipcams",
        .desc = "Flip the camera order",
        .type = OptionType::Bool,
        .game_name = "Beatmania IIDX",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "IIDX Disable Cameras",
        .name = "iidxdisablecams",
        .desc = "Disables cameras",
        .type = OptionType::Bool,
        .game_name = "Beatmania IIDX",
        .category = "Game Options",
    },
    {
        .title = "IIDX Sound Output Device",
        .name = "iidxsounddevice",
        .desc =
            "SOUND_OUTPUT_DEVICE environment variable override. Default: auto. "
            "Auto will use WASAPI, unless -iidxasio is set or if XONAR SOUND CARD is present, then ASIO is used",
        .type = OptionType::Enum,
        .game_name = "Beatmania IIDX",
        .category = "Game Options",
        .elements = {
            {"auto", "Automatic"},
            {"wasapi", "Windows Audio"},
            {"asio", "ASIO"},
        },
    },
    {
        .title = "IIDX ASIO Driver",
        .name = "iidxasio",
        .desc = "ASIO driver name to use, replacing XONAR SOUND CARD(64). "
            "String should match registry key under HKLM\\SOFTWARE\\ASIO\\. "
            "IIDX is very picky about ASIO devices it can support, YMMV",
        .type = OptionType::Text,
        .game_name = "Beatmania IIDX",
        .category = "Game Options",
    },
    {
        .title = "IIDX BIO2 Firmware",
        .name =  "iidxbio2fw",
        .desc = "Enables BIO2 firmware updates",
        .type = OptionType::Bool,
        .game_name = "Beatmania IIDX",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "IIDX TDJ Mode",
        .name =  "iidxtdj",
        .desc = "Enables TDJ cabinet mode. Ensure you also set -iidxsounddevice to desired option",
        .type = OptionType::Bool,
        .game_name = "Beatmania IIDX",
        .category = "Game Options",
    },
    {
        // spice2x_IIDXDigitalTTSensitivity
        .title = "IIDX Digital TT Sensitivity",
        .name = "sp2x-iidxdigitalttsens",
        .desc = "Adjust sensitivity of turntable when digital input (buttons) is used. Does not affect analog input! Default: 4",
        .type = OptionType::Integer,
        .setting_name = "(0-255)",
        .game_name = "Beatmania IIDX",
        .category = "Game Options (Advanced)",
    },
    {
        // spice2x_IIDXLDJForce720p
        .title = "IIDX LDJ Force 720p (HD)",
        .name = "sp2x-iidxldjforce720p",
        .desc = "Force Definition Type HD (720p) mode instead of FHD (1080p) for IIDX30+. Only for LDJ! TDJ is always FHD in IIDX30+",
        .type = OptionType::Bool,
        .game_name = "Beatmania IIDX",
        .category = "Game Options (Advanced)",
    },
    {
        // spice2x_IIDXTDJSubSize
        .title = "IIDX TDJ Subscreen Size",
        .name = "sp2x-iidxtdjsubsize",
        .desc = "Default size of the subscreen overlay. Default: medium",
        .type = OptionType::Enum,
        .game_name = "Beatmania IIDX",
        .category = "Overlay",
        .elements = {
            {"small", ""},
            {"medium", ""},
            {"large", ""},
            {"fullscreen", ""},
        },
    },
    {
        // spice2x_IIDXLEDFontSize
        .title = "IIDX LED Ticker Font Size",
        .name = "sp2x-iidxledfontsize",
        .desc = "Configure the font size of the segment display, in pixels. Default: 64 (px)",
        .type = OptionType::Integer,
        .game_name = "Beatmania IIDX",
        .category = "Overlay",
    },
    {
        // spice2x_IIDXLEDColor
        .title = "IIDX LED Ticker Color",
        .name = "sp2x-iidxledcolor",
        .desc = "Configure the font color of the segment display; specify RGB value in hex. "
                "Default: 0xff0000 (red)",
        .type = OptionType::Hex,
        .game_name = "Beatmania IIDX",
        .category = "Overlay",
    },
    {
        // spice2x_IIDXLEDPos
        .title = "IIDX LED Ticker Position",
        .name = "sp2x-iidxledpos",
        .desc = "Initial position of the segment display. Default: bottom",
        .type = OptionType::Enum,
        .game_name = "Beatmania IIDX",
        .category = "Overlay",
        .elements = {
            {"topleft", ""},
            {"top", ""},
            {"topright", ""},
            {"bottomleft", ""},
            {"bottom", ""},
            {"bottomright", ""},
        },
    },
    {
        .title = "Force Load Sound Voltex Module",
        .name = "sdvx",
        .desc = "Manually enable Sound Voltex Module",
        .type = OptionType::Bool,
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Force 720p",
        .name = "sdvx720",
        .desc = "Force Sound Voltex 720p display mode, used by older games. "
            "For newer games, use window resize function instead",
        .type = OptionType::Bool,
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Printer Emulation",
        .name = "printer",
        .desc = "Enable Sound Voltex printer emulation",
        .type = OptionType::Bool,
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Printer Output Path",
        .name = "printerpath",
        .desc = "Path to folder where images will be stored",
        .type = OptionType::Text,
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Printer Output Clear",
        .name = "printerclear",
        .desc = "Clean up saved images in the output directory on startup",
        .type = OptionType::Bool,
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Printer Output Overwrite",
        .name = "printeroverwrite",
        .desc = "Always overwrite the same file in output directory",
        .type = OptionType::Bool,
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Printer Output Format",
        .name = "printerformat",
        .desc = "Path to folder where images will be stored",
        .type = OptionType::Text,
        .setting_name = "(png/bmp/tga/jpg)",
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Printer JPG Quality",
        .name = "printerjpgquality",
        .desc = "Quality setting in percent if JPG format is used",
        .type = OptionType::Integer,
        .setting_name = "(0-100)",
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "SDVX Disable Cameras",
        .name = "sdvxdisablecams",
        .desc = "Disables cameras",
        .type = OptionType::Bool,
        .game_name = "Sound Voltex",
        .category = "Game Options",
    },
    {
        .title = "SDVX Native Touch Handling",
        .name = "sdvxnativetouch",
        .desc = "Disables touch hooks and lets the game access a touch screen directly. "
                "Requires a touch screen to be connected as a secondary monitor. "
                "Enable this when you get duplicate touch inputs from an actual touch screen",
        .type = OptionType::Bool,
        .game_name = "Sound Voltex",
        .category = "Game Options",
    },
    {
        // spice2x_SDVXDigitalKnobSensitivity
        .title = "SDVX Digital Knob Sensitivity",
        .name = "sp2x-sdvxdigitalknobsens",
        .desc = "Adjust sensitivity of knobs when digital input (buttons) is used. Does not affect analog input! Default: 16",
        .type = OptionType::Integer,
        .setting_name = "(0-255)",
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        // spice2x_SDVXAsioDriver
        .title = "SDVX ASIO driver",
        .name = "sp2x-sdvxasio",
        .desc = "ASIO driver name to use, replacing XONAR SOUND CARD(64). "
            "String should match registry key under HLKM\\SOFTWARE\\ASIO\\ "
            "SDVX is EXTREMELY picky about ASIO devices it can support!",
        .type = OptionType::Text,
        .game_name = "Sound Voltex",
        .category = "Game Options (Advanced)",
    },
    {
        // spice2x_SDVXSubPos
        .title = "SDVX Subscreen Overlay Position",
        .name = "sp2x-sdvxsubpos",
        .desc = "Location for the subscreen overlay. Default: bottom",
        .type = OptionType::Enum,
        .game_name = "Sound Voltex",
        .category = "Overlay",
        .elements = {{"top", ""}, {"center", ""}, {"bottom", ""}},
    },
    {
        .title = "Force Load DDR Module",
        .name = "ddr",
        .desc = "Manually enable Dance Dance Revolution module",
        .type = OptionType::Bool,
        .game_name = "Dance Dance Revolution",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "DDR 4:3 Mode",
        .name = "ddrsd/o",
        .desc = "Enable DDR 4:3 (SD) mode",
        .type = OptionType::Bool,
        .game_name = "Dance Dance Revolution",
        .category = "Game Options",
    },
    {
        .title = "Force Load Pop'n Music Module",
        .name = "pnm",
        .desc = "Manually enable Pop'n Music module",
        .type = OptionType::Bool,
        .game_name = "Pop'n Music",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Pop'n Music Force HD Mode",
        .name = "pnmhd",
        .desc = "Force enable Pop'n Music HD mode",
        .type = OptionType::Bool,
        .game_name = "Pop'n Music",
        .category = "Game Options",
    },
    {
        .title = "Pop'n Music Force SD Mode",
        .name = "pnmsd",
        .desc = "Force enable Pop'n Music SD mode",
        .type = OptionType::Bool,
        .game_name = "Pop'n Music",
        .category = "Game Options",
    },
    {
        .title = "Force Load HELLO! Pop'n Music Module",
        .name = "hpm",
        .desc = "Manually enable HELLO! Pop'n Music module",
        .type = OptionType::Bool,
        .game_name = "HELLO! Pop'n Music",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load GitaDora Module",
        .name = "gd",
        .desc = "Manually enable GitaDora module",
        .type = OptionType::Bool,
        .game_name = "GitaDora",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "GitaDora Two Channel Audio",
        .name = "2ch",
        .desc = "Manually enable GitaDora module",
        .type = OptionType::Bool,
        .game_name = "GitaDora",
        .category = "Game Options",
    },
    {
        .title = "GitaDora Cabinet Type",
        .name = "gdcabtype",
        .desc = "Select cabinet type. DX has more input and lights. Pick SD2 for single player guitar mode on white cab",
        .type = OptionType::Enum,
        .game_name = "GitaDora",
        .category = "Game Options",
        .elements = {{"0", "DX (broken?)"}, {"1", "DX"}, {"2", "SD"}, {"3", "SD2 (white cab)"}},
    },
    {
        .title = "Force Load Jubeat Module",
        .name = "jb",
        .desc = "Manually enable Jubeat module",
        .type = OptionType::Bool,
        .game_name = "Jubeat",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Reflec Beat Module",
        .name = "rb",
        .desc = "Manually enable Reflec Beat module",
        .type = OptionType::Bool,
        .game_name = "Reflec Beat",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Tenkaichi Shogikai Module",
        .name = "shogikai",
        .desc = "Manually enable Tenkaichi Shogikai module",
        .type = OptionType::Bool,
        .game_name = "Tenkaichi Shogikai",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Beatstream Module",
        .name = "bs",
        .desc = "Manually enable Beatstream module",
        .type = OptionType::Bool,
        .game_name = "Beatstream",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Nostalgia Module",
        .name = "nostalgia",
        .desc = "Manually enable Nostalgia module",
        .type = OptionType::Bool,
        .game_name = "Nostalgia",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Dance Evolution Module",
        .name = "dea",
        .desc = "Manually enable Dance Evolution module",
        .type = OptionType::Bool,
        .game_name = "Dance Evolution",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load FutureTomTom Module",
        .name = "ftt",
        .desc = "Manually enable FutureTomTom module",
        .type = OptionType::Bool,
        .game_name = "FutureTomTom",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load BBC Module",
        .name = "bbc",
        .desc = "Manually enable Bishi Bashi Channel module",
        .type = OptionType::Bool,
        .game_name = "Bishi Bashi Channel",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Metal Gear Arcade Module",
        .name = "mga",
        .desc = "Manually enable Metal Gear Arcade module",
        .type = OptionType::Bool,
        .game_name = "Metal Gear",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Quiz Magic Academy Module",
        .name = "qma",
        .desc = "Manually enable Quiz Magic Academy module",
        .type = OptionType::Bool,
        .game_name = "Quiz Magic Academy",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Road Fighters 3D Module",
        .name = "rf3d",
        .desc = "Manually enable Road Fighters 3D module",
        .type = OptionType::Bool,
        .game_name = "Road Fighters 3D",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Steel Chronicle Module",
        .name = "sc",
        .desc = "Manually enable Steel Chronicle module",
        .type = OptionType::Bool,
        .game_name = "Steel Chronicle",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Mahjong Fight Club Module",
        .name = "mfc",
        .desc = "Manually enable Mahjong Fight Club module",
        .type = OptionType::Bool,
        .game_name = "Mahjong Fight Club",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Scotto Module",
        .name = "scotto",
        .desc = "Manually enable Scotto module",
        .type = OptionType::Bool,
        .game_name = "Scotto",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Dance Rush Module",
        .name = "dr",
        .desc = "Manually enable Dance Rush module",
        .type = OptionType::Bool,
        .game_name = "DANCERUSH",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Winning Eleven Module",
        .name = "we",
        .desc = "Manually enable Winning Eleven module",
        .type = OptionType::Bool,
        .game_name = "Winning Eleven",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Otoca D'or Module",
        .name = "otoca",
        .desc = "Manually enable Otoca D'or module",
        .type = OptionType::Bool,
        .game_name = "Otoca D'or",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load LovePlus Module",
        .name = "loveplus",
        .desc = "manually enable LovePlus module",
        .type = OptionType::Bool,
        .game_name = "LovePlus",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Charge Machine Module",
        .name = "pcm",
        .desc = "manually enable Charge Machine module",
        .type = OptionType::Bool,
        .game_name = "Charge Machine",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Ongaku Paradise Module",
        .name = "onpara",
        .desc = "manually enable Ongaku Paradise module",
        .type = OptionType::Bool,
        .game_name = "Ongaku Paradise",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Force Load Busou Shinki Module",
        .name = "busou",
        .desc = "manually enable Busou Shinki module",
        .type = OptionType::Bool,
        .game_name = "Busou Shinki: Armored Princess Battle Conductor",
        .category = "Game Options (Advanced)",
    },
    {
        .title = "Modules Folder Path",
        .name = "modules",
        .desc = "Sets a custom path to the modules folder",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Screenshot Folder Path",
        .name = "screenshotpath",
        .desc = "Sets a custom path to the screenshots folder",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Configuration Path",
        .name = "cfgpath",
        .desc = "Sets a custom path for config.xml",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Intel SDE Folder Path",
        .name = "sde",
        .desc = "Path to Intel SDE kit path for automatic attaching",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Path to ea3-config.xml",
        .name = "e",
        .desc = "Sets a custom path to ea3-config.xml",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Path to app-config.xml",
        .name = "a",
        .desc = "Sets a custom path to app-config.xml",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Path to avs-config.xml",
        .name = "v",
        .desc = "Sets a custom path to avs-config.xml",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Path to bootstrap.xml",
        .name = "b",
        .desc = "Sets a custom path to bootstrap.xml",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "Path to log.txt",
        .name = "y",
        .desc = "Sets a custom path to log.txt",
        .type = OptionType::Text,
        .category = "Paths",
    },
    {
        .title = "API TCP Port",
        .name = "api",
        .desc = "Port the API should be listening on",
        .type = OptionType::Integer,
        .category = "SpiceCompanion and API",
    },
    {
        .title = "API Password",
        .name = "apipass",
        .desc = "Set the custom user password needed to use the API",
        .type = OptionType::Text,
        .category = "SpiceCompanion and API",
        .sensitive = true,
    },
    {
        .title = "API Verbose Logging",
        .name = "apilogging",
        .desc = "verbose logging of API activity",
        .type = OptionType::Bool,
        .category = "SpiceCompanion and API (Advanced)",
    },
    {
        .title = "API Serial Port",
        .name = "apiserial",
        .desc = "Serial port the API should be listening on",
        .type = OptionType::Text,
        .category = "SpiceCompanion and API (Advanced)",
    },
    {
        .title = "API Serial Baud",
        .name = "apiserialbaud",
        .desc = "Baud rate for the serial port",
        .type = OptionType::Integer,
        .category = "SpiceCompanion and API (Advanced)",
    },
    {
        .title = "API Pretty",
        .name = "apipretty",
        .desc = "Slower, but pretty API output",
        .type = OptionType::Bool,
        .category = "SpiceCompanion and API (Advanced)",
    },
    {
        .title = "API Debug Mode",
        .name = "apidebug",
        .desc = "Enables API debugging mode",
        .type = OptionType::Bool,
        .category = "SpiceCompanion and API (Advanced)",
    },
    {
        .title = "Enable All IO Modules",
        .name = "io",
        .desc = "Manually enable ALL IO emulation",
        .type = OptionType::Bool,
        .category = "I/O Modules",
    },
    {
        .title = "Enable ACIO Module",
        .name = "acio",
        .desc = "Manually enable ACIO emulation",
        .type = OptionType::Bool,
        .category = "I/O Modules",
    },
    {
        .title = "Enable ICCA Module",
        .name = "icca",
        .desc = "Manually enable ICCA emulation",
        .type = OptionType::Bool,
        .category = "I/O Modules",
    },
    {
        .title = "Enable DEVICE Module",
        .name = "device",
        .desc = "Manually enable DEVICE emulation",
        .type = OptionType::Bool,
        .category = "I/O Modules",
    },
    {
        .title = "Enable EXTDEV Module",
        .name = "extdev",
        .desc = "Manually enable EXTDEV emulation",
        .type = OptionType::Bool,
        .category = "I/O Modules",
    },
    {
        .title = "Enable SCIUNIT Module",
        .name = "sciunit",
        .desc = "Manually enable SCIUNIT emulation",
        .type = OptionType::Bool,
        .category = "I/O Modules",
    },
    {
        .title = "Enable device passthrough",
        .name = "devicehookdisable",
        .desc = "Disable I/O and serial device hooks",
        .type = OptionType::Bool,
        .category = "I/O Modules",
    },
    {
        .title = "Disable Raw Input Touch",
        .name = "wintouch",
        .desc = "For touch screen input, disable usage of Raw Input API and instead use "
                "Win8 Pointer API or Win7 Touch API. Only enable this if you have trouble "
                "using the default (raw input) touch input, as Raw Input performs better",
        .type = OptionType::Bool,
        .category = "Touch Parameters",
    },
    {
        .title = "Force Touch Emulation",
        .name = "touchemuforce",
        .desc = "Force enable hook for GetTouchInputInfo API and inject WM_TOUCH events. "
                "This is automatically turned on when needed, so it is not typically required to "
                "turn it on manually. Do not enable this unless you have trouble",
        .type = OptionType::Bool,
        .category = "Touch Parameters",
    },
    {
        .title = "Invert Raw Input Touch",
        .name = "touchinvert",
        .desc = "Inverts raw touch coordinates; only works for default raw input handler, "
                "and not Windows Touch API",
        .type = OptionType::Bool,
        .category = "Touch Parameters",
    },
    {
        .title = "Disable Touch Card Insert",
        .name = "touchnocard",
        .desc = "Disables touch overlay card insert button",
        .type = OptionType::Bool,
        .category = "Touch Parameters",
    },
    {
        .title = "ICCA Reader Port",
        .name = "reader",
        .desc = "Connects to and uses a ICCA on a given COM port",
        .type = OptionType::Text,
        .category = "Card Readers",
    },
    {
        .title = "ICCA Reader Port (with toggle)",
        .name = "togglereader",
        .desc = "Connects to and uses a ICCA on a given COM port, and enabled NumLock toggling between P1/P2",
        .type = OptionType::Text,
        .category = "Card Readers",
    },
    {
        .title = "CardIO HID Reader Support",
        .name = "cardio",
        .desc = "Enables detection and support of cardio HID readers",
        .type = OptionType::Bool,
        .category = "Card Readers",
    },
    {
        .title = "CardIO HID Reader Order Flip",
        .name = "cardioflip",
        .desc = "Flips the order of detection for P1/P2",
        .type = OptionType::Bool,
        .category = "Card Readers",
    },
    {
        .title = "HID SmartCard",
        .name = "scard",
        .desc = "Detects and uses HID smart card readers for card input",
        .type = OptionType::Bool,
        .category = "Card Readers",
    },
    {
        .title = "HID SmartCard Order Flip",
        .name = "scardflip",
        .desc = "Flips the order of detection for P1/P2",
        .type = OptionType::Bool,
        .category = "Card Readers",
    },
    {
        .title = "HID SmartCard Order Toggle",
        .name = "scardtoggle",
        .desc = "Toggles reader between P1/P2 using the NumLock key state",
        .type = OptionType::Bool,
        .category = "Card Readers",
    },
    {
        .title = "SextetStream Port",
        .name = "sextet",
        .desc = "Use a SextetStream device on a given COM port",
        .type = OptionType::Text,
        .category = "Card Readers",
    },
    {
        .title = "Enable BemaniTools 5 API",
        .name = "bt5api",
        .desc = "Enables partial BemaniTools 5 API compatibility layer",
        .type = OptionType::Bool,
        .category = "Miscellaneous",
    },
    {
        .title = "Realtime Process Priority",
        .name = "realtime",
        .desc = "Sets the process priority to realtime, can help with odd lag spikes",
        .type = OptionType::Bool,
        .hidden = true,
        .category = "Performance",
    },
    {
        // spice2x_ProcessPriority
        .title = "Process Priority",
        .name = "sp2x-processpriority",
        .desc = "Sets the process priority, can help with odd lag spikes. Default: high",
        .type = OptionType::Enum,
        .category = "Performance",
        .elements = {
            {"belownormal", ""},
            {"normal", ""},
            {"abovenormal", ""},
            {"high", ""},
            {"realtime", ""}
        },
    },
    {
        // spice2x_ProcessAffinity
        .title = "Process Affinity",
        .name = "sp2x-processaffinity",
        .desc = "Set process affinity by calling SetProcessAffinityMask. "
                "Must provide a hexadecimal mask (e.g., 0x1ff00)",
        .type = OptionType::Hex,
        .category = "Performance",
    },
    {
        .title = "Heap Size",
        .name = "h",
        .desc = "Custom heap size in bytes",
        .type = OptionType::Integer,
        .category = "Performance",
    },
    // TODO: remove this and create an ignore list
    {
        .title = "(REMOVED) Disable G-Sync Detection",
        .name = "keepgsync",
        .desc = "Broken feature that was not implemented correctly",
        .type = OptionType::Bool,
        .hidden = true,
        .category = "Graphics (Common)",
    },
    {
        .title = "Disable Spice Audio Hook",
        .name = "audiohookdisable",
        .desc = "Disables audio device initialization hooks",
        .type = OptionType::Bool,
        .category = "Audio",
    },
    {
        .title = "Spice Audio Hook Backend",
        .name = "audiobackend",
        .desc = "Selects the audio backend to use when spice audio hook is enabled. "
            "Game must be outputting to WASAPI Exclusive mode for audio hook to work.",
        .type = OptionType::Enum,
        .category = "Audio",
        .elements = {{"asio", "ASIO"}, {"waveout", "waveOut"}},
    },
    {
        .title = "Spice Audio Hook ASIO Driver ID",
        .name = "asiodriverid",
        .desc = "Selects the ASIO driver id to use when Spice Audio Backend is set to ASIO",
        .type = OptionType::Integer,
        .category = "Audio",
    },
    {
        .title = "WASAPI Dummy Context",
        .name = "audiodummy",
        .desc = "Uses a dummy `IAudioClient` context to maintain full audio control",
        .type = OptionType::Bool,
        .category = "Audio",
    },
    {
        // DelayBy5Seconds
        .title = "Delay by 5 Seconds",
        .name = "sleep",
        .desc = "Waits five seconds before starting the game",
        .type = OptionType::Bool,
        .hidden = true, // superseded by sp2x-sleep_duration
        .category = "Miscellaneous",
    },
    {
        // spice2x_DelayByNSeconds
        .title = "Delay Game Launch",
        .name = "sp2x-sleepduration",
        .desc = "Wait for N seconds before starting the game",
        .type = OptionType::Integer,
        .category = "Miscellaneous",
    },
    {
        .title = "Load Stubs",
        .name = "stubs",
        .desc = "Enables loading kbt/kld stub files",
        .type = OptionType::Bool,
        .category = "Miscellaneous",
    },
    {
        .title = "Adjust Display Orientation",
        .name = "adjustorientation",
        .desc = "Automatically adjust the orientation of your display in portrait games. "
                "WARNING: game may launch at incorrect refresh rate! Use in combination with "
                "-graphics-force-refresh flag to fix.",
        .type = OptionType::Bool,
        .hidden = true, // superseded by sp2x-autoorientation
        .category = "Graphics (Common)",
    },
    {
        // spice2x_AutoOrientation
        .title = "Auto-rotate Display",
        .name = "sp2x-autoorientation",
        .desc = "Automatically adjust the orientation of your display when launched. "
                "WARNING: game may launch at incorrect refresh rate! Use in combination with "
                "-graphics-force-refresh and potentially -9on12 to fix.",
        .type = OptionType::Enum,
        .category = "Graphics (Common)",
        // match graphics_orientation
        .elements = {{"0", "90 (CW)"}, {"1", "270 (CCW)"}},
    },
    {
        .title = "Log Level",
        .name = "loglevel",
        .desc = "Set the level of detail that gets written to the log",
        .type = OptionType::Enum,
        .category = "Performance",
        .elements = {{"fatal", ""}, {"warning", ""}, {"info", ""}, {"misc", ""}, {"all", ""}, {"disable", ""}},
    },
    {
        .title = "EA Automap",
        .name = "automap",
        .desc = "Enable automap in patch configuration",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "EA Netdump",
        .name = "netdump",
        .desc = "Enable automap in network dumping configuration",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Discord RPC AppID Override",
        .name = "discordappid",
        .desc = "Set the discord RPC AppID override",
        .type = OptionType::Text,
        .category = "Development",
    },
    {
        .title = "Blocking Logger",
        .name = "logblock",
        .desc = "Slower but safer logging used for debugging",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Debug CreateFile",
        .name = "createfiledebug",
        .desc = "Outputs CreateFile debug prints",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Verbose Graphics Logging",
        .name = "graphicsverbose",
        .desc = "Enable the verbose logging of graphics hook code",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Verbose AVS Logging",
        .name = "avsverbose",
        .desc = "Enable the verbose logging of AVS filesystem functions",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Disable Colored Output",
        .name = "nocolor",
        .desc = "Disable terminal colors for log outputs to console",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Disable ACP Hook",
        .name = "acphookdisable",
        .desc = "Force disable advanced code pages hooks for encoding",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Disable Signal Handling",
        .name = "signaldisable",
        .desc = "Force disable signal handling",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Disable Debug Hooks",
        .name = "dbghookdisable",
        .desc = "Disable hooks for debug functions (e.g. OutputDebugString)",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Disable AVS VFS Drive Mount Redirection",
        .name = "avs-redirect-disable",
        .desc = "Disable D:/E:/F: AVS VFS mount redirection",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        .title = "Output PEB",
        .name = "pebprint",
        .desc = "Prints PEB on startup to console",
        .type = OptionType::Bool,
        .category = "Development",
    },
    {
        // spice2x_LightsOverallBrightness
        .title = "Lights Brightness Adjustment",
        .name = "sp2x-lights-brightness",
        .desc = "Modify HID output values, measured in %. Default is 100. "
                "Requires compatible hardware, not just any LED",
        .type = OptionType::Integer,
        .setting_name = "(0-255)",
        .category = "Miscellaneous",
    },
    {
        // spice2x_WindowBorder
        .title = "Window Border Style",
        .name = "sp2x-windowborder",
        .desc = "For windowed mode: overrides window border style. "
            "Does not work for all games; can possibly make the game crash! "
            "Can also be changed in Screen Resize window (default F11)",
        .type = OptionType::Enum,
        .category = "Graphics (Windowed)",
        // match cfg::WindowDecorationMode
        .elements = {
            {"0", "default"},
            {"1", "borderless"},
            {"2", "resizable window"},
        },
    },
    {
        // spice2x_WindowSize
        .title = "Window Size",
        .name = "sp2x-windowsize",
        .desc = "For windowed mode: override for window size, width and height separated by comma. "
            "Can also be changed in Screen Resize window (default F11)",
        .type = OptionType::Text,
        .setting_name = "1280,720",
        .category = "Graphics (Windowed)",
    },
    {
        // spice2x_WindowPosition
        .title = "Window Position",
        .name = "sp2x-windowpos",
        .desc = "For windowed mode: override for window position, x and y separated by comma. "
            "Can also be changed in Screen Resize window (default F11)",
        .type = OptionType::Text,
        .setting_name = "120,240",
        .category = "Graphics (Windowed)",
    },
    {
        // spice2x_WindowAlwaysOnTop
        .title = "Window Always on Top",
        .name = "sp2x-windowalwaysontop",
        .desc = "For windowed mode: make window to be always on top of other windows. "
            "Can also be changed in Screen Resize window (default F11)",
        .type = OptionType::Bool,
        .category = "Graphics (Windowed)",
    },
    {
        // spice2x_JubeatLegacyTouch
        .title = "Legacy Touch Targets",
        .name = "sp2x-jubeatlegacytouch",
        .desc = "For touch screen players - use the legacy & less accurate grid-based layout for touch recognition, "
            "instead of the new & more accurate touch targets. Default: off",
        .type = OptionType::Bool,
        .game_name = "Jubeat",
        .category = "Game Options",
    },
    {
        // spice2x_RBTouchScale
        .title = "Scale Touch Input",
        .name = "sp2x-rbscaletouch",
        .desc = "Apply scaling to make touch area smaller than the screen. "
            "Specify a number out of 1000 (e.g., 800 means 80%). Default: 1000",
        .type = OptionType::Integer,
        .game_name = "Reflec Beat",
        .category = "Game Options",
    },
};

const std::vector<std::string> &launcher::get_categories(bool advanced) {
    return advanced ? CATEGORY_ORDER_ADVANCED : CATEGORY_ORDER_BASIC;
}

const std::vector<OptionDefinition> &launcher::get_option_definitions() {
    return OPTION_DEFINITIONS;
}

std::unique_ptr<std::vector<Option>> launcher::parse_options(int argc, char *argv[]) {

    // generate options
    auto &definitions = get_option_definitions();
    auto options = std::make_unique<std::vector<Option>>();

    options->reserve(definitions.size());

    for (auto &definition : definitions) {

        // create aliases
        std::vector<std::string> aliases;
        strsplit(definition.name, aliases, '/');

        // create option
        auto &option = options->emplace_back(definition, "");

        // check if enabled
        for (int i = 1; i < argc; i++) {

            // ignore leading '-' characters
            auto argument = argv[i];
            while (argument[0] == '-') {
                argument++;
            }

            // check aliases
            for (const auto &alias : aliases) {
                if (_stricmp(alias.c_str(), argument) == 0) {
                    switch (definition.type) {
                        case OptionType::Bool: {
                            option.value_add("/ENABLED");
                            break;
                        }
                        case OptionType::Integer: {
                            if (++i >= argc) {
                                log_fatal("options", "missing parameter for -{}", alias);
                            } else {
                                // validate it is an integer
                                char *p;
                                strtol(argv[i], &p, 10);
                                if (*p) {
                                    log_fatal("options", "parameter for -{} is not a number: {}", alias, argv[i]);
                                } else {
                                    option.value_add(argv[i]);
                                }
                            }
                            break;
                        }
                        case OptionType::Hex: {
                            if (++i >= argc) {
                                log_fatal("options", "missing parameter for -{}", alias);
                            } else {
                                // validate it is an integer
                                try {
                                    std::stoull(argv[i], nullptr, 16);
                                } catch (const std::exception &ex) {
                                    log_fatal("options", "parameter for -{} is not a hex number: {}", alias, argv[i]);
                                }
                            }
                            break;
                        }
                        case OptionType::Enum:
                        case OptionType::Text: {
                            if (++i >= argc) {
                                log_fatal("options", "missing parameter for -{}", alias);
                            } else {
                                option.value_add(argv[i]);
                            }
                            break;
                        }
                        default: {
                            log_warning("options", "unknown option type: {} (-{})", definition.type, alias);
                            break;
                        }
                    }
                    break;
                }
            }
        }
    }

    // positional arguments
    std::vector<std::string> positional;
    for (int i = 1; i < argc; i++) {

        // check if enabled
        bool found = false;
        for (auto &definition : definitions) {

            // create aliases
            std::vector<std::string> aliases;
            strsplit(definition.name, aliases, '/');

            // ignore leading '-' characters
            auto argument = argv[i];
            while (argument[0] == '-') {
                argument++;
            }

            // check aliases
            for (const auto &alias : aliases) {
                if (_stricmp(alias.c_str(), argument) == 0) {
                    found = true;
                    switch (definition.type) {
                        case OptionType::Bool:
                            break;
                        case OptionType::Integer:
                        case OptionType::Hex:
                        case OptionType::Text:
                        case OptionType::Enum:
                            i++;
                            break;
                    }
                    break;
                }
            }

            // early quit
            if (found) {
                break;
            }
        }

        // positional argument
        if (!found && *argv[i] != '-') {
            positional.emplace_back(argv[i]);
        }
    }

    // game executable
    if (!positional.empty()) {
        options->at(launcher::Options::GameExecutable).value = positional[0];
    }

    // return vector
    return options;
}

std::vector<Option> launcher::merge_options(
        const std::vector<Option> &options,
        const std::vector<Option> &overrides)
{
    std::vector<Option> merged;

    for (const auto &option : options) {
        for (const auto &override : overrides) {
            if (option.get_definition().name == override.get_definition().name) {
                if (override.is_active()) {
                    if (option.is_active()) {
                        auto &new_option = merged.emplace_back(option.get_definition(), "");
                        new_option.disabled = true;

                        for (auto &value : option.values()) {
                            new_option.value_add(value);
                        }
                        for (auto &value : override.values()) {
                            new_option.value_add(value);
                        }
                    } else {
                        auto &new_option = merged.emplace_back(override.get_definition(), "");
                        new_option.disabled = true;

                        for (auto &value : override.values()) {
                            new_option.value_add(value);
                        }
                    }
                } else {
                    merged.push_back(option);
                }
                break;
            }
        }
    }

    return merged;
}

std::string launcher::detect_bootstrap_release_code() {
    std::string bootstrap_path = "prop/bootstrap.xml";

    // load XML
    tinyxml2::XMLDocument bootstrap;
    if (bootstrap.LoadFile(bootstrap_path.c_str()) != tinyxml2::XML_SUCCESS) {
        log_warning("options", "unable to parse {}", bootstrap_path);
        return "";
    }

    // find release_code
    auto node_root = bootstrap.LastChild();
    if (node_root) {
        auto node_release_code = node_root->FirstChildElement("release_code");
        if (node_release_code) {
            return node_release_code->GetText();
        }
    }

    // failure
    log_warning("options", "no release_code found in {}", bootstrap_path);
    return "";
}

static launcher::GameVersion detect_gameversion_ident() {

    // detect ea3-ident path
    std::string ident_path;
    if (fileutils::file_exists("prop/ea3-ident.xml")) {
        ident_path = "prop/ea3-ident.xml";
    }
    if (ident_path.empty()) {
        log_warning("options", "unable to detect ea3-ident.xml file");
        return launcher::GameVersion();
    }

    // load XML
    tinyxml2::XMLDocument ea3_ident;
    if (ea3_ident.LoadFile(ident_path.c_str()) != tinyxml2::XML_SUCCESS) {
        log_warning("options", "unable to parse {}", ident_path);
        return launcher::GameVersion();
    }

    // find model string
    auto node_root = ea3_ident.LastChild();
    if (node_root) {
        auto node_soft = node_root->FirstChildElement("soft");
        if (node_soft) {
            launcher::GameVersion version;
            bool error = true;
            auto node_model = node_soft->FirstChildElement("model");
            if (node_model) {
                version.model = node_model->GetText();
                error = false;
            }
            auto node_dest = node_soft->FirstChildElement("dest");
            if (node_dest) {
                version.dest = node_dest->GetText();
            }
            auto node_spec = node_soft->FirstChildElement("spec");
            if (node_spec) {
                version.spec = node_spec->GetText();
            }
            auto node_rev = node_soft->FirstChildElement("rev");
            if (node_rev) {
                version.rev = node_rev->GetText();
            }
            auto node_ext = node_soft->FirstChildElement("ext");
            if (node_ext) {
                version.ext = node_ext->GetText();
                auto bootstrap_ext = launcher::detect_bootstrap_release_code();
                if (version.ext.size() != 10 && bootstrap_ext.size() == 10) {
                    version.ext = bootstrap_ext;
                } else if (bootstrap_ext.size() == 10) {
                    int ext_cur = 0;
                    int ext_new = 0;
                    try {
                        ext_cur = std::stoi(version.ext);
                        try {
                            ext_new = std::stoi(bootstrap_ext);
                            if (ext_new > ext_cur) {
                                version.ext = bootstrap_ext;
                            }
                        } catch (const std::exception &ex) {
                            log_warning("options", "unable to parse soft/ext: {}", bootstrap_ext);
                        }
                    } catch (const std::exception &ex) {
                        log_warning("options", "unable to parse soft/ext: {}", version.ext);
                        version.ext = bootstrap_ext;
                    }
                }
            }
            if (!error) {
                log_info("options", "using model {}:{}:{}:{}:{} from {}",
                         version.model, version.dest, version.spec, version.rev, version.ext,
                         ident_path);
                return version;
            }
        }
    }

    // error
    log_warning("options", "unable to find /ea3_conf/soft/model in {}", ident_path);
    return launcher::GameVersion();
}

launcher::GameVersion launcher::detect_gameversion(const std::string &ea3_user) {

    // detect ea3-config path
    std::string ea3_path;
    if (!ea3_user.empty()) {
        ea3_path = ea3_user;
    } else if (fileutils::file_exists("prop/ea3-config.xml")) {
        ea3_path = "prop/ea3-config.xml";
    } else if (fileutils::file_exists("prop/ea3-cfg.xml")) {
        ea3_path = "prop/ea3-cfg.xml";
    } else if (fileutils::file_exists("prop/eamuse-config.xml")) {
        ea3_path = "prop/eamuse-config.xml";
    }
    if (ea3_path.empty()) {
        log_warning("options", "unable to detect EA3 configuration file");
        return detect_gameversion_ident();
    }

    // load XML
    tinyxml2::XMLDocument ea3_config;
    if (ea3_config.LoadFile(ea3_path.c_str()) != tinyxml2::XML_SUCCESS) {
        log_warning("options", "unable to parse {}", ea3_path);
        log_warning("options", "trying again with improper XML header removed...");

        // below: dirty hack to deal with bad ea3 xml file with comments in the beginning
        // https://stackoverflow.com/questions/19100408/tinyxml-any-way-to-skip-problematic-doctype-tag

        // Open the file and read it into a vector
        std::ifstream ifs(ea3_path.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        std::ifstream::pos_type fsize = ifs.tellg();
        ifs.seekg(0, std::ios::beg);
        std::vector<char> bytes(fsize);
        ifs.read(&bytes[0], fsize);

        // Create string from vector
        std::string xml_str(&bytes[0], fsize);

        // Skip unsupported statements
        size_t pos = 0;
        while (true) {
            pos = xml_str.find_first_of("<", pos);
            if (xml_str[pos + 1] == '?' || // <?xml...
                xml_str[pos + 1] == '!') { // <!DOCTYPE... or [<!ENTITY...
                // Skip this line
                pos = xml_str.find_first_of("\n", pos);
            } else
                break;
        }
        xml_str = xml_str.substr(pos);

        // replace with fixed one
        ea3_config.Clear();
        if (ea3_config.Parse(xml_str.c_str()) != tinyxml2::XML_SUCCESS) {
            // if we still failed, give up
            log_warning("options", "unable to parse fixed xml");
            return detect_gameversion_ident();
        }
    }

    // find model string
    auto node_root = ea3_config.LastChild();
    if (node_root) {
        auto node_soft = node_root->FirstChildElement("soft");
        if (node_soft) {
            GameVersion version;
            bool error = true;
            auto node_model = node_soft->FirstChildElement("model");
            if (node_model) {
                version.model = node_model->GetText();
                error = false;
            }
            auto node_dest = node_soft->FirstChildElement("dest");
            if (node_dest) {
                version.dest = node_dest->GetText();
            }
            auto node_spec = node_soft->FirstChildElement("spec");
            if (node_spec) {
                version.spec = node_spec->GetText();
            }
            auto node_rev = node_soft->FirstChildElement("rev");
            if (node_rev) {
                version.rev = node_rev->GetText();
            }
            auto node_ext = node_soft->FirstChildElement("ext");
            if (node_ext) {
                version.ext = node_ext->GetText();
                auto bootstrap_ext = launcher::detect_bootstrap_release_code();
                if (version.ext.size() != 10 && bootstrap_ext.size() == 10) {
                    version.ext = bootstrap_ext;
                } else if (bootstrap_ext.size() == 10) {
                    int ext_cur = 0;
                    int ext_new = 0;
                    try {
                        ext_cur = std::stoi(version.ext);
                        try {
                            ext_new = std::stoi(bootstrap_ext);
                            if (ext_new > ext_cur) {
                                version.ext = bootstrap_ext;
                            }
                        } catch (const std::exception &ex) {
                            log_warning("options", "unable to parse soft/ext: {}", bootstrap_ext);
                        }
                    } catch (const std::exception &ex) {
                        log_warning("options", "unable to parse soft/ext: {}", version.ext);
                        version.ext = bootstrap_ext;
                    }
                }
            }
            if (!error) {
                log_info("options", "using model {}:{}:{}:{}:{} from {}",
                         version.model, version.dest, version.spec, version.rev, version.ext,
                         ea3_path);
                return version;
            }
        }
    }

    // error
    log_warning("options", "unable to find /ea3/soft/model in {}", ea3_path);
    return detect_gameversion_ident();
}
