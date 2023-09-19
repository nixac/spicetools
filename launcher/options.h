#pragma once

#include <memory>
#include <vector>

#include "cfg/option.h"

namespace launcher {

    // options list - order matters
    namespace Options {
        enum {
            GameExecutable,
            OpenConfigurator,
            OpenKFControl,
            EAmusementEmulation,
            ServiceURL,
            PCBID,
            Player1Card,
            Player2Card,
            WindowedMode,
            InjectHook,
            ExecuteScript,
            CaptureCursor,
            ShowCursor,
            DisplayAdapter,
            GraphicsForceRefresh,
            GraphicsForceSingleAdapter,
            Graphics9On12,
            NoLegacy,
            RichPresence,
            SmartEAmusement,
            EAmusementMaintenance,
            AdapterNetwork,
            AdapterSubnet,
            DisableNetworkFixes,
            HTTP11,
            DisableSSL,
            URLSlash,
            SOFTID,
            VREnable,
            DisableOverlay,
            spice2x_FpsAutoShow,
            spice2x_SubScreenAutoShow,
            spice2x_IOPanelAutoShow,
            spice2x_KeypadAutoShow,
            LoadIIDXModule,
            IIDXCameraOrderFlip,
            IIDXDisableCameras,
            IIDXSoundOutputDevice,
            IIDXAsioDriver,
            IIDXBIO2FW,
            IIDXTDJMode,
            spice2x_IIDXDigitalTTSensitivity,
            spice2x_IIDXLDJForce720p,
            spice2x_IIDXTDJSubSize,
            spice2x_IIDXLEDFontSize,
            spice2x_IIDXLEDColor,
            spice2x_IIDXLEDPos,
            LoadSoundVoltexModule,
            SDVXForce720p,
            SDVXPrinterEmulation,
            SDVXPrinterOutputPath,
            SDVXPrinterOutputClear,
            SDVXPrinterOutputOverwrite,
            SDVXPrinterOutputFormat,
            SDVXPrinterJPGQuality,
            SDVXDisableCameras,
            SDVXNativeTouch,
            spice2x_SDVXDigitalKnobSensitivity,
            spice2x_SDVXAsioDriver,
            spice2x_SDVXSubPos,
            LoadDDRModule,
            DDR43Mode,
            LoadPopnMusicModule,
            PopnMusicForceHDMode,
            PopnMusicForceSDMode,
            LoadHelloPopnMusicModule,
            LoadGitaDoraModule,
            GitaDoraTwoChannelAudio,
            GitaDoraCabinetType,
            LoadJubeatModule,
            LoadReflecBeatModule,
            LoadShogikaiModule,
            LoadBeatstreamModule,
            LoadNostalgiaModule,
            LoadDanceEvolutionModule,
            LoadFutureTomTomModule,
            LoadBBCModule,
            LoadMetalGearArcadeModule,
            LoadQuizMagicAcademyModule,
            LoadRoadFighters3DModule,
            LoadSteelChronicleModule,
            LoadMahjongFightClubModule,
            LoadScottoModule,
            LoadDanceRushModule,
            LoadWinningElevenModule,
            LoadOtocaModule,
            LoadLovePlusModule,
            LoadChargeMachineModule,
            LoadOngakuParadiseModule,
            LoadBusouShinkiModule,
            PathToModules,
            ScreenshotFolder,
            ConfigurationPath,
            IntelSDEFolder,
            PathToEa3Config,
            PathToAppConfig,
            PathToAvsConfig,
            PathToBootstrap,
            PathToLog,
            APITCPPort,
            APIPassword,
            APIVerboseLogging,
            APISerialPort,
            APISerialBaud,
            APIPretty,
            APIDebugMode,
            EnableAllIOModules,
            EnableACIOModule,
            EnableICCAModule,
            EnableDEVICEModule,
            EnableEXTDEVModule,
            EnableSCIUNITModule,
            EnableDevicePassthrough,
            ForceWinTouch,
            ForceTouchEmulation,
            InvertTouchCoordinates,
            DisableTouchCardInsert,
            ICCAReaderPort,
            ICCAReaderPortToggle,
            CardIOHIDReaderSupport,
            CardIOHIDReaderOrderFlip,
            HIDSmartCard,
            HIDSmartCardOrderFlip,
            HIDSmartCardOrderToggle,
            SextetStreamPort,
            EnableBemaniTools5API,
            RealtimeProcessPriority,
            spice2x_ProcessPriority,
            spice2x_ProcessAffinity,
            HeapSize,
            DisableGSyncDetection,
            DisableAudioHooks,
            AudioBackend,
            AsioDriverId,
            AudioDummy,
            DelayBy5Seconds,
            spice2x_DelayByNSeconds,
            LoadStubs,
            AdjustOrientation,
            spice2x_AutoOrientation,
            LogLevel,
            EAAutomap,
            EANetdump,
            DiscordAppID,
            BlockingLogger,
            DebugCreateFile,
            VerboseGraphicsLogging,
            VerboseAVSLogging,
            DisableColoredOutput,
            DisableACPHook,
            DisableSignalHandling,
            DisableDebugHooks,
            DisableAvsVfsDriveMountRedirection,
            OutputPEB,
            spice2x_LightsOverallBrightness,
            spice2x_WindowBorder,
            spice2x_WindowSize,
            spice2x_WindowPosition,
            spice2x_WindowAlwaysOnTop,
            spice2x_JubeatLegacyTouch,
            spice2x_RBTouchScale,
        };
    }

    const std::vector<std::string> &get_categories(bool advanced);
    const std::vector<OptionDefinition> &get_option_definitions();
    std::unique_ptr<std::vector<Option>> parse_options(int argc, char *argv[]);
    std::vector<Option> merge_options(const std::vector<Option> &options, const std::vector<Option> &overrides);

    struct GameVersion {
        std::string model;
        std::string dest;
        std::string spec;
        std::string rev;
        std::string ext;
    };

    std::string detect_bootstrap_release_code();
    GameVersion detect_gameversion(const std::string& ea3_user);
}
