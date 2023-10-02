#include "pipewire.h"
#include "hooks/audio/audio.h"
#include "hooks/audio/backends/wasapi/audio_client.h"
#include "util/libutils.h"
#include "launcher/launcher.h"

/* Imports (refer to bmsound-wine.dll.spec) */
typedef void(*BmswClientInit_t)(const char*);

const WAVEFORMATEXTENSIBLE &PipewireBackend::format() const noexcept {
    return format_;
}

HRESULT PipewireBackend::on_initialize(
    AUDCLNT_SHAREMODE *ShareMode,
    DWORD *StreamFlags,
    REFERENCE_TIME *hnsBufferDuration,
    REFERENCE_TIME *hnsPeriodicity,
    const WAVEFORMATEX *pFormat,
    LPCGUID AudioSessionGuid) noexcept
{
    log_info("audio::pipewire", "on_initialize");

    // WASAPI init
    *ShareMode = AUDCLNT_SHAREMODE_SHARED;
    *StreamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
            AUDCLNT_STREAMFLAGS_RATEADJUST |
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
            AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
    *hnsBufferDuration = 100000;
    *hnsPeriodicity = 100000;

    // Start pipewire thread
    if (!bmsw_) log_fatal("audio::pipewire", "Library not found: '{}'", (MODULE_PATH / "bmsound-wine.dll").string());
    auto pipewireinit = (BmswClientInit_t)GetProcAddress(bmsw_, "BmswClientInit");
    if (!pipewireinit) log_fatal("audio::pipewire", "Library invalid: '{}'", (MODULE_PATH / "bmsound-wine.dll").string());
    internal_ = std::thread(pipewireinit,"GAME TITLE");

    return S_OK;
}
HRESULT PipewireBackend::on_get_buffer_size(uint32_t *buffer_frames) noexcept {
    *buffer_frames = 0;

    return S_OK;
}
HRESULT PipewireBackend::on_get_stream_latency(REFERENCE_TIME *latency) noexcept {
    *latency = 100000;

    return S_OK;
}
HRESULT PipewireBackend::on_get_current_padding(std::optional<uint32_t> &padding_frames) noexcept {

    padding_frames = 0;

    return S_OK;
}
HRESULT PipewireBackend::on_is_format_supported(
    AUDCLNT_SHAREMODE *ShareMode,
    const WAVEFORMATEX *pFormat,
    WAVEFORMATEX **ppClosestMatch) noexcept
{
    // only accept 44.1 kHz, stereo, 16-bits per channel
    if (*ShareMode == AUDCLNT_SHAREMODE_EXCLUSIVE &&
        pFormat->nChannels == 2 &&
        pFormat->nSamplesPerSec == 44100 &&
        pFormat->wBitsPerSample == 16)
    {
        return S_OK;
    }

    return AUDCLNT_E_UNSUPPORTED_FORMAT;
}
HRESULT PipewireBackend::on_get_mix_format(WAVEFORMATEX **pp_device_format) noexcept {
    return E_NOTIMPL;
}
HRESULT PipewireBackend::on_get_device_period(
    REFERENCE_TIME *default_device_period,
    REFERENCE_TIME *minimum_device_period)
{
    *default_device_period = 10000;
    *minimum_device_period = 10000;

    return S_OK;
}
HRESULT PipewireBackend::on_start() noexcept {
    return S_OK;
}
HRESULT PipewireBackend::on_stop() noexcept {
    return S_OK;
}
HRESULT PipewireBackend::on_set_event_handle(HANDLE *event_handle) {

    *event_handle = CreateEvent(nullptr, true, false, nullptr);

    return S_OK;
}

HRESULT PipewireBackend::on_get_buffer(uint32_t num_frames_requested, BYTE **ppData) {
    static BYTE buf[10000];
    *ppData = buf;

    return S_OK;
}
HRESULT PipewireBackend::on_release_buffer(uint32_t num_frames_written, DWORD dwFlags) {

    return S_OK;
}
PipewireBackend::PipewireBackend() : format_(hooks::audio::FORMAT), bmsw_(libutils::try_library(MODULE_PATH / "bmsound-wine.dll"))
{

}
