#include "none.h"
#include "hooks/audio/audio.h"
#include "hooks/audio/backends/wasapi/audio_client.h"


const WAVEFORMATEXTENSIBLE &NoneBackend::format() const noexcept {
    return format_;
}

HRESULT NoneBackend::on_initialize(
    AUDCLNT_SHAREMODE *ShareMode,
    DWORD *StreamFlags,
    REFERENCE_TIME *hnsBufferDuration,
    REFERENCE_TIME *hnsPeriodicity,
    const WAVEFORMATEX *pFormat,
    LPCGUID AudioSessionGuid) noexcept
{
    *ShareMode = AUDCLNT_SHAREMODE_SHARED;
    *StreamFlags = AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
            AUDCLNT_STREAMFLAGS_RATEADJUST |
            AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
            AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
    *hnsBufferDuration = 100000;
    *hnsPeriodicity = 100000;
    log_info("audio::none", "on_initialize");

    return S_OK;
}
HRESULT NoneBackend::on_get_buffer_size(uint32_t *buffer_frames) noexcept {
    *buffer_frames = 0;

    return S_OK;
}
HRESULT NoneBackend::on_get_stream_latency(REFERENCE_TIME *latency) noexcept {
    *latency = 100000;

    return S_OK;
}
HRESULT NoneBackend::on_get_current_padding(std::optional<uint32_t> &padding_frames) noexcept {

    padding_frames = 0;

    return S_OK;
}
HRESULT NoneBackend::on_is_format_supported(
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
HRESULT NoneBackend::on_get_mix_format(WAVEFORMATEX **pp_device_format) noexcept {
    return E_NOTIMPL;
}
HRESULT NoneBackend::on_get_device_period(
    REFERENCE_TIME *default_device_period,
    REFERENCE_TIME *minimum_device_period)
{
    *default_device_period = 10000;
    *minimum_device_period = 10000;

    return S_OK;
}
HRESULT NoneBackend::on_start() noexcept {
    return S_OK;
}
HRESULT NoneBackend::on_stop() noexcept {
    return S_OK;
}
HRESULT NoneBackend::on_set_event_handle(HANDLE *event_handle) {

    *event_handle = CreateEvent(nullptr, true, false, nullptr);

    return S_OK;
}

HRESULT NoneBackend::on_get_buffer(uint32_t num_frames_requested, BYTE **ppData) {
    static BYTE buf[10000];
    *ppData = buf;

    return S_OK;
}
HRESULT NoneBackend::on_release_buffer(uint32_t num_frames_written, DWORD dwFlags) {

    return S_OK;
}
NoneBackend::NoneBackend() : format_(hooks::audio::FORMAT)
{

}
