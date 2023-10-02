#pragma once

#include "backend.h"
#include <thread>


struct PipewireBackend final : AudioBackend {
public:
    explicit PipewireBackend();
    ~PipewireBackend() final = default;

    [[nodiscard]] const WAVEFORMATEXTENSIBLE &format() const noexcept override;

    HRESULT on_initialize(
        AUDCLNT_SHAREMODE *ShareMode,
        DWORD *StreamFlags,
        REFERENCE_TIME *hnsBufferDuration,
        REFERENCE_TIME *hnsPeriodicity,
        const WAVEFORMATEX *pFormat,
        LPCGUID AudioSessionGuid) noexcept override;

    HRESULT on_get_buffer_size(uint32_t *buffer_frames) noexcept override;
    HRESULT on_get_stream_latency(REFERENCE_TIME *latency) noexcept override;
    HRESULT on_get_current_padding(std::optional<uint32_t> &padding_frames) noexcept override;

    HRESULT on_is_format_supported(
        AUDCLNT_SHAREMODE *ShareMode,
        const WAVEFORMATEX *pFormat,
        WAVEFORMATEX **ppClosestMatch) noexcept override;

    HRESULT on_get_mix_format(WAVEFORMATEX **pp_device_format) noexcept override;

    HRESULT on_get_device_period(
        REFERENCE_TIME *default_device_period,
        REFERENCE_TIME *minimum_device_period) override;

    HRESULT on_start() noexcept override;
    HRESULT on_stop() noexcept override;
    HRESULT on_set_event_handle(HANDLE *event_handle) override;

    HRESULT on_get_buffer(uint32_t num_frames_requested, BYTE **ppData) override;
    HRESULT on_release_buffer(uint32_t num_frames_written, DWORD dwFlags) override;

private:

    const WAVEFORMATEXTENSIBLE &format_;
    std::thread internal_;
    HMODULE bmsw_;
    //BYTE *active_sound_buffer = nullptr;
};

