#include <winternl.h>
#include "pipewire.h"
#include "hooks/audio/audio.h"
#include "hooks/audio/backends/wasapi/audio_client.h"
#include "util/libutils.h"
#include "launcher/launcher.h"
#include "hooks/audio/util.h"
#include "hooks/audio/backends/wasapi/util.h"


/*
... ShareMode           : AUDCLNT_SHAREMODE_EXCLUSIVE <- backend to reimplement, THIS is the audio engine, GAME is client https://learn.microsoft.com/en-us/windows/win32/coreaudio/audclnt-streamflags-xxx-constants
... StreamFlags         : AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST <- first is the whole reason for relay_handle_ requirement, second flag not tested
... hnsBufferDuration   : 10000 <- wasapi people inventing new time units, 1ms (since 1unit==100ns, 10000*0.0001ms)
... hnsPeriodicity      : 10000 <- same as above (1ms)
... nChannels           : 2 <- channel_count
... nSamplesPerSec      : 44100 <- bitrate
... nAvgBytesPerSec     : 176400 <- raw buffer size per second (bitrate * stride)(bytes)
... nBlockAlign         : 4 <- stride (bytes)
... wBitsPerSample      : 16 <- sample size (bits)
... wValidBitsPerSample : 16 <- same as above
... dwChannelMask       : SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT <- position config
 _INFO: Code pipeline
  INIT: on_is_format_supported()?->on_initialize()->on_set_event_handle()-:(LOOPx1):->on_start()
  LOOP: if relay_handle_: on_get_buffer_size()->on_get_buffer()->..->on_release_buffer()
  DEINIT: on_stop()

 _REV: POSSIBLE ALTERNATIVES:
  Check if the stream is driving. The stream needs to have the
  * PW_STREAM_FLAG_DRIVER set. When the stream is driving,
  * pw_stream_trigger_process() needs to be called when data is
  * available (output) or needed (input). Since 0.3.34
  bool pw_stream_is_driving(struct pw_stream *stream);
 * */

/* Imports/Exports (refer to bmsound-wine.dll.spec) */
typedef void (*BmswConfigInit_t)(const char *);
typedef void (*BmswExperimentalForceProfile_t)(const char *);
typedef int(*BmswClientFormatIsSupported_t)(DWORD, DWORD, DWORD, void *);
typedef int(*BmswClientFormatPeriodFPC_t)(void *);
typedef REFERENCE_TIME(*BmswClientFormatPeriodWRT_t)(void *);
typedef void *(*BmswClientCreate_t)(const char *, void *, void *);
typedef int(*BmswClientStart_t)(void *);
typedef int(*BmswClientStop_t)(void *);
typedef int(*BmswClientDestroy_t)(void *);
typedef unsigned char *(*BmswClientGetBuffer_t)(void *, uint32_t);
typedef int(*BmswClientReleaseBuffer_t)(void *, uint32_t);
typedef void (*BmswClientUpdateCallback_t)(void *, void *, void *);
static BmswConfigInit_t BmswConfigInit;
[[maybe_unused]] static BmswExperimentalForceProfile_t BmswExperimentalForceProfile;
static BmswClientFormatIsSupported_t BmswClientFormatIsSupported;
static BmswClientFormatPeriodFPC_t BmswClientFormatPeriodFPC;
static BmswClientFormatPeriodWRT_t BmswClientFormatPeriodWRT;
static BmswClientCreate_t BmswClientCreate;
static BmswClientStart_t BmswClientStart;
static BmswClientStop_t BmswClientStop;
static BmswClientDestroy_t BmswClientDestroy;
static BmswClientGetBuffer_t BmswClientGetBuffer;
static BmswClientReleaseBuffer_t BmswClientReleaseBuffer;
[[maybe_unused]] static BmswClientUpdateCallback_t BmswClientUpdateCallback;
static HMODULE bmsw_ = nullptr;

/* Audio init (unless specified otherwise, run once at start) */
// Reports to game whether each requested audio format is available for device (first success return will be used)
HRESULT PipewireBackend::on_is_format_supported(AUDCLNT_SHAREMODE *ShareMode, const WAVEFORMATEX *pFormat, WAVEFORMATEX **ppClosestMatch) noexcept
{
    // Format reporting and filtering
    log_info("audio::pipewire", "{}", __FUNCTION__);
    log_misc("audio::pipewire", "Checking backend support for {} channels, {} Hz, {}-bit",
             pFormat->nChannels,
             pFormat->nSamplesPerSec,
             pFormat->wBitsPerSample);

    // IIDX? will always request format that was last checked through this function
    if (*ShareMode != AUDCLNT_SHAREMODE_EXCLUSIVE) return AUDCLNT_E_UNSUPPORTED_FORMAT;

    // Request format support from real backend (only accepts 44.1 kHz, stereo, 16-bits per channel for now)
    return BmswClientFormatIsSupported(pFormat->nSamplesPerSec, pFormat->nChannels, pFormat->wBitsPerSample, nullptr) == 0 ? S_OK : AUDCLNT_E_UNSUPPORTED_FORMAT;
}
// Populate hnsPeriodicity, _INFO: runs before on_initialize, requires configured client stream data
HRESULT PipewireBackend::on_get_device_period(REFERENCE_TIME *default_device_period, REFERENCE_TIME *minimum_device_period)
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    *default_device_period = wrt_;
    *minimum_device_period = wrt_;

    return S_OK;
}
// This expects to be run once on_is_format_supported() succeeds, initializes backend based on passed arguments (and updates them if needed)
HRESULT PipewireBackend::on_initialize(AUDCLNT_SHAREMODE *ShareMode, DWORD *StreamFlags, REFERENCE_TIME *hnsBufferDuration, REFERENCE_TIME *hnsPeriodicity, const WAVEFORMATEX *pFormat, LPCGUID AudioSessionGuid) noexcept
{
    log_info("audio::pipewire", "{}", __FUNCTION__);

    // Initialize pipewire client (without starting the thread)
    client_ = BmswClientCreate(GAME_INSTANCE->title(), nullptr, nullptr);//(void *)callback_notify,this

    if (!client_)
    log_fatal("audio::pipewire", "Client could not be initialized");
    log_info("audio::pipewire", "Client initialized: '{}'", fmt::ptr(client_));

    // Adjust WASAPI configuration visible to game (passed arguments are populated and should match that of last on_is_format_supported call)
    *hnsBufferDuration = wrt_;
    *hnsPeriodicity = wrt_;

    //_TODO: Init info
    log_info("audio::asio", "Device Info:");
    log_info("audio::pipewire", "... hnsBufferDuration : {}", *hnsBufferDuration);
    log_info("audio::pipewire", "... hnsPeriodicity    : {}", *hnsPeriodicity);

    return S_OK;
}
// This takes ownership over shared event handle exclusive to AUDCLNT_STREAMFLAGS_EVENTCALLBACK _INFO: first loop iteration runs directly after this, before on_start() call
HRESULT PipewireBackend::on_set_event_handle(HANDLE *event_handle)
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    this->relay_handle_ = *event_handle; // take over WASAPI's owned handle pre-initialized by client
    //BmswClientUpdateCallback(client_, (void *) 0, this->relay_handle_);
    *event_handle = CreateEvent(nullptr, true, false, nullptr); // replace WASAPI's handle with always off handle

    return S_OK;

    ULONG ntbuf_size = sizeof(OBJECT_NAME_INFORMATION) + 1024;
    char ntbuf[ntbuf_size];
    OBJECT_NAME_INFORMATION *ntinfo = (OBJECT_NAME_INFORMATION *) ntbuf;
    NTSTATUS status = NtQueryObject(*event_handle, ObjectNameInformation, ntinfo, ntbuf_size, &ntbuf_size);
    if (NT_SUCCESS(status))
    {
        // possible to rename anonymous handle to access by name
    }
}
HRESULT PipewireBackend::on_start() noexcept
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    BmswClientStart(client_);
    return S_OK;
}
PipewireBackend::PipewireBackend() : relay_handle_(nullptr), format_(hooks::audio::FORMAT), client_(nullptr)
{
    log_info("audio::pipewire", "{}", __FUNCTION__);

    // Initialize bmsound-wine.dll once
    if (!bmsw_ && (bmsw_ = libutils::try_library(MODULE_PATH / "bmsound-wine.dll")))
    {
        BmswConfigInit = (BmswConfigInit_t) GetProcAddress(bmsw_, "BmswConfigInit");
        BmswExperimentalForceProfile = (BmswExperimentalForceProfile_t) GetProcAddress(bmsw_, "BmswExperimentalForceProfile");
        BmswClientFormatIsSupported = (BmswClientFormatIsSupported_t) GetProcAddress(bmsw_, "BmswClientFormatIsSupported");
        BmswClientFormatPeriodWRT = (BmswClientFormatPeriodWRT_t) GetProcAddress(bmsw_, "BmswClientFormatPeriodWRT");
        BmswClientFormatPeriodFPC = (BmswClientFormatPeriodFPC_t) GetProcAddress(bmsw_, "BmswClientFormatPeriodFPC");
        BmswClientCreate = (BmswClientCreate_t) GetProcAddress(bmsw_, "BmswClientCreate");
        BmswClientStart = (BmswClientStart_t) GetProcAddress(bmsw_, "BmswClientStart");
        BmswClientStop = (BmswClientStop_t) GetProcAddress(bmsw_, "BmswClientStop");
        BmswClientDestroy = (BmswClientDestroy_t) GetProcAddress(bmsw_, "BmswClientDestroy");
        BmswClientGetBuffer = (BmswClientGetBuffer_t) GetProcAddress(bmsw_, "BmswClientGetBuffer");
        BmswClientReleaseBuffer = (BmswClientReleaseBuffer_t) GetProcAddress(bmsw_, "BmswClientReleaseBuffer");
        BmswClientUpdateCallback = (BmswClientUpdateCallback_t) GetProcAddress(bmsw_, "BmswClientUpdateCallback");

        // Load config
        BmswConfigInit("prop/linux.json");
        //BmswExperimentalForceProfile("notif_spice");
    }
    if (bmsw_)
    {
        // Sync config
        wrt_ = BmswClientFormatPeriodWRT(nullptr); //_INFO: wrt affects reported latency (1:100ns)
        fpc_ = BmswClientFormatPeriodFPC(nullptr);
        return;
    }
    log_fatal("audio::pipewire", "Library not found: '{}'", (MODULE_PATH / "bmsound-wine.dll").string());
}

/* Audio callback loop (reacts to state of relay_handle_, most likely separate thread */
// _REV: Synchronizing client and backend with nanotime may be necessary (bmsound-pw endpoint dependant)
inline static void callback_notify(void *self)
{
    //log_info("audio::pipewire", "{}", __FUNCTION__);
    auto *self_ = (PipewireBackend *) self;
    //std::this_thread::sleep_for(std::chrono::nanoseconds(10 * 1000 * 1000 - 227));
    if (!SetEvent(self_->relay_handle_)) // has to be called for game-side audio loop to continue
    {
        DWORD last_error = GetLastError();

        log_warning("audio::asio", "AsioBackend::buffer_switch: SetEvent failed: {} ({})",
                    last_error,
                    std::system_category().message(last_error));
    }
}
// Amount in frames of data we may handle at once (which should always end up being what gets send by client, unless overrun happens)
HRESULT PipewireBackend::on_get_buffer_size(uint32_t *buffer_frames) noexcept
{
    static int iterc = -1;
    if (iterc == -1)
    {
        log_info("audio::pipewire", "{}, frames: {} (INITIAL HIT)", __FUNCTION__, fpc_);
        iterc = 0;
    }

    *buffer_frames = fpc_;

    return S_OK;
}
// Wants a raw stream buffer to be assigned into *ppData, this stream will have x amount of sound frames stored into it by a client and assumes exclusive access until next on_release_buffer
HRESULT PipewireBackend::on_get_buffer(uint32_t num_frames_requested, BYTE **ppData)
{
    static int iterc = -1;
    if (iterc == -1)
    {
        log_info("audio::pipewire", "{}, frames: {} (INITIAL HIT)", __FUNCTION__, num_frames_requested);
    }
    iterc++;
    if (iterc > 999999)
    {
        log_info("audio::pipewire", "on_get_buffer, frames: {} (HIT {})", num_frames_requested, iterc);
        iterc = 0;
    }

    *ppData = BmswClientGetBuffer(client_, num_frames_requested);

    return S_OK;
}
// This releases access to buffer from last on_get_buffer and implies x amount of frames being valid data to be streamed
HRESULT PipewireBackend::on_release_buffer(uint32_t num_frames_written, DWORD dwFlags)
{
    static int iterc = -1;
    if (iterc == -1)
    {
        log_info("audio::pipewire", "{}, frames: {} (INITIAL HIT)", __FUNCTION__, num_frames_written);
        iterc = 0;
    }

    BmswClientReleaseBuffer(client_, num_frames_written);
    callback_notify(this);

    return S_OK;
}

/* Audio deinit (unless specified otherwise, run once at termination) */
HRESULT PipewireBackend::on_stop() noexcept
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    BmswClientStop(client_);
    return S_OK;
}
PipewireBackend::~PipewireBackend()
{
    log_info("audio::pipewire", "~PipewireBackend");
    if (client_) BmswClientDestroy(client_);
}

/* Unsorted (unknown callers, should be considered as unimplemented/untested) *///_BUG: does this need implementation?
const WAVEFORMATEXTENSIBLE &PipewireBackend::format() const noexcept
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    return format_;
}
HRESULT PipewireBackend::on_get_stream_latency(REFERENCE_TIME *latency) noexcept
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    *latency = BmswClientFormatPeriodWRT(client_);

    return S_OK;
}
HRESULT PipewireBackend::on_get_current_padding(std::optional<uint32_t> &padding_frames) noexcept
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    padding_frames = 0;

    return S_OK;
}
HRESULT PipewireBackend::on_get_mix_format(WAVEFORMATEX **pp_device_format) noexcept
{
    log_info("audio::pipewire", "{}", __FUNCTION__);
    return E_NOTIMPL;
}
