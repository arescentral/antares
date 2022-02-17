// Copyright (C) 1997, 1999-2001, 2008 Nathan Lamont
// Copyright (C) 2022 The Antares Authors
//
// This file is part of Antares, a tactical space combat game.
//
// Antares is free software: you can redistribute it and/or modify it
// under the terms of the Lesser GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Antares is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Antares.  If not, see http://www.gnu.org/licenses/

#include "sound/xaudio2-driver.hpp"

#include "data/audio.hpp"
#include "data/resource.hpp"

#include <pn/output>
#include <stdexcept>
#include <assert.h>


#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <xaudio2.h>

using std::unique_ptr;

namespace antares {

namespace {

BYTE* convert_to_stereo(const BYTE* input_data, size_t num_channels, size_t num_samples) {
    BYTE*          output_data = new BYTE[num_samples * 2 * sizeof(int16_t)];
    int16_t*       output_data_i16 = reinterpret_cast<int16_t*>(output_data);
    const int16_t* input_data_i16  = reinterpret_cast<const int16_t*>(input_data);

    if (num_channels == 1) {
        for (size_t i = 0; i < num_samples; i++) {
            output_data_i16[i * 2 + 0] = output_data_i16[i * 2 + 1] = input_data_i16[i];
        }
    } else if (num_channels == 2) {
        memcpy(output_data, input_data, num_samples * 2 * sizeof(int16_t));
    } else {
        for (size_t i = 0; i < num_samples; i++) {
            output_data_i16[i * 2 + 0] = input_data_i16[i * num_channels + 0];
            output_data_i16[i * 2 + 1] = input_data_i16[i * num_channels + 1];
        }
    }
    return output_data;
}

void check_hresult(pn::string_view method, HRESULT hresult) {
    if (FAILED(hresult)) {
        throw std::runtime_error(
                pn::format("{0}: {1}", method, static_cast<int>(hresult)).c_str());
    }
}

}  // namespace

// Garbage collectable objects can cause expensive stalls on the thread deleting them, so their deletion
// is deferred to a separate cleanup thread
class XAudio2SoundDriver::XAudio2GarbageCollectable {
  public:
    XAudio2GarbageCollectable() : _next_collectable(nullptr){}

    virtual ~XAudio2GarbageCollectable() {}

    XAudio2GarbageCollectable* get_next_gc() const { return _next_collectable; }
    void set_next_gc(XAudio2GarbageCollectable* next) { _next_collectable = next; }

  private:
    XAudio2GarbageCollectable* _next_collectable;
};



class XAudio2SoundDriver::XAudio2SoundInstance : public XAudio2GarbageCollectable {
  public:
    XAudio2SoundInstance(XAudio2SoundDriver& driver, BYTE* data, size_t data_size)
            : _driver(driver), _data(data), _data_size(data_size), _ref_count(1) {}

    ~XAudio2SoundInstance() { delete[] _data; }

    void AddRef() { InterlockedIncrement(&_ref_count); }

    void Release() {
        if (InterlockedDecrement(&_ref_count) == 0) {
            _driver.garbage_collect(this);
        }
    }

    BYTE* get_data() const { return _data; }
    size_t get_data_size() const { return _data_size; }

  private:
    XAudio2SoundDriver& _driver;
    size_t              _data_size;
    BYTE*               _data;
    volatile LONG       _ref_count;
};

class XAudio2SoundDriver::XAudio2VoiceInstance : public XAudio2GarbageCollectable {
  public:
    XAudio2VoiceInstance(XAudio2SoundDriver& driver, IXAudio2Voice* voice)
            : _driver(driver), _voice(voice) {}

    ~XAudio2VoiceInstance() { _voice->DestroyVoice(); }

  protected:
    XAudio2SoundDriver& _driver;
    IXAudio2Voice*      _voice;
};

class XAudio2SoundDriver::XAudio2SourceVoiceInstance : public XAudio2GarbageCollectable {
  public:
    XAudio2SourceVoiceInstance(XAudio2SoundDriver& driver, IXAudio2SourceVoice* voice)
            : _driver(driver),
              _voice(voice),
              _active_sound(nullptr),
              _active_is_loop(false),
              _pending_sound(nullptr),
              _pending_looping(false),
              _source_voice_state(SourceVoiceState::Idle)
    {
        InitializeCriticalSection(&_mutex);
    }

    ~XAudio2SourceVoiceInstance() {
        reset_and_play_sound(nullptr, false);

        _voice->DestroyVoice();

        DeleteCriticalSection(&_mutex);

        assert(_active_sound == nullptr);
    }

    IXAudio2SourceVoice* get_voice() const { _voice; }

    void on_buffer_end();
    void reset_and_play_sound(XAudio2SoundInstance* sound, bool looping);

  private:
    // To simplify XAudio2 voice state handling, we only ever submit one buffer at a time.
    enum class SourceVoiceState {
        Idle,                     // No buffers are submitted, _active_sound and _pending_sound are both null.
        Playing,                  // A buffer was submitted and will eventually play.
        Resetting,                // A buffer was submitted, but a stop was also submitted.  OnBufferEnd will transition to pending state.
    };

    void submit_active_sound();
    void set_and_submit_sound(XAudio2SoundInstance* sound, bool looping);

    XAudio2SoundDriver&   _driver;

    IXAudio2SourceVoice*  _voice;

    // Mutex-guarded fields
    CRITICAL_SECTION      _mutex;
    XAudio2SoundInstance* _pending_sound;
    bool                  _pending_looping;

    XAudio2SoundInstance* _active_sound;
    bool                  _active_is_loop;

    SourceVoiceState      _source_voice_state;
};

void XAudio2SoundDriver::XAudio2SourceVoiceInstance::on_buffer_end() {
    EnterCriticalSection(&_mutex);
    if (_source_voice_state == SourceVoiceState::Playing) {
        if (_active_is_loop) {
            submit_active_sound();
        } else {
            _source_voice_state = SourceVoiceState::Resetting;
        }
    }

    if (_source_voice_state == SourceVoiceState::Resetting) {
        _active_sound->Release();

        _active_sound       = _pending_sound;
        _active_is_loop     = _pending_looping;

        _pending_sound = nullptr;
        _pending_looping = false;

        if (_active_sound) {
            _source_voice_state = SourceVoiceState::Playing;
            submit_active_sound();
        } else {
            _source_voice_state = SourceVoiceState::Idle;
        }
    }
    LeaveCriticalSection(&_mutex);
}


void XAudio2SoundDriver::XAudio2SourceVoiceInstance::reset_and_play_sound(
        XAudio2SoundInstance* sound, bool looping) {
    if (sound) {
        sound->AddRef();
    }

    EnterCriticalSection(&_mutex);

    switch (_source_voice_state) {
        case SourceVoiceState::Idle:
            // No sound is submitted, submit this one
            assert(pending_sound == nullptr);
            assert(_active_sound == nullptr);
            if (sound) {
                _source_voice_state = SourceVoiceState::Playing;
                set_and_submit_sound(sound, looping);
            }
        break;
        case SourceVoiceState::Playing:
            // A sound was submitted, stop it and change to the specified one
            if (_pending_sound) {
                _pending_sound->Release();
            }
            _pending_sound      = sound;
            _source_voice_state = SourceVoiceState::Resetting;
            _voice->Stop(0, 0);
            _voice->FlushSourceBuffers();
            break;
        case SourceVoiceState::Resetting:
            // A sound was submitted but a stop was already requested, wait for it
            if (_pending_sound) {
                _pending_sound->Release();
            }
            _pending_sound = sound;
            break;
        default: break;
    }
    _pending_looping = looping;

    LeaveCriticalSection(&_mutex);
}

void XAudio2SoundDriver::XAudio2SourceVoiceInstance::submit_active_sound() {
    XAUDIO2_BUFFER buffer;
    buffer.Flags = 0;
    buffer.AudioBytes = _active_sound->get_data_size();
    buffer.pAudioData = _active_sound->get_data();
    buffer.PlayBegin  = 0;
    buffer.PlayLength = 0;
    buffer.LoopBegin  = 0;
    buffer.LoopLength = 0;
    buffer.LoopCount  = 0;
    buffer.pContext   = this;

    if (FAILED(_voice->SubmitSourceBuffer(&buffer, nullptr))) {
        _active_sound->Release();
        _active_sound       = nullptr;
        _source_voice_state = SourceVoiceState::Idle;
    } else {
        _voice->Start(0, 0);
    }
}

void XAudio2SoundDriver::XAudio2SourceVoiceInstance::set_and_submit_sound(
    XAudio2SoundInstance* sound, bool looping) {
    _active_sound = sound;
    _active_is_loop = looping;
    submit_active_sound();
}

class XAudio2SoundDriver::XAudio2Sound : public Sound {
  public:
    XAudio2Sound(XAudio2SoundDriver& driver)
            : _driver(driver), _instance(nullptr), _sample_rate(1) {}

    ~XAudio2Sound() {
        if (_instance)
            _instance->Release();
    }

    virtual void play(uint8_t volume);
    virtual void loop(uint8_t volume);

    void buffer(const SoundData& s) {
        if (_instance) {
            _instance->Release();
            _instance = nullptr;
        }

        if (s.channels == 0) {
            return;
        }

        const size_t num_samples = s.data.size() / s.channels / sizeof(int16_t);
        BYTE* data = convert_to_stereo(
                reinterpret_cast<const BYTE*>(s.data.data()), s.channels, num_samples);

        size_t data_size = num_samples * 2 * sizeof(int16_t);

        _instance = new XAudio2SoundInstance(_driver, data, data_size);
        _sample_rate = s.frequency;
    }

    XAudio2SoundInstance* sound_instance() const { return _instance; }

  private:
    XAudio2SoundDriver&       _driver;
    XAudio2SoundInstance*     _instance;
    uint32_t                  _sample_rate;
};

class XAudio2SoundDriver::XAudio2VoiceCallbacks : public IXAudio2VoiceCallback {
  public:
    void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32 BytesRequired) override {}
    void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
    void STDMETHODCALLTYPE OnStreamEnd() override {}
    void STDMETHODCALLTYPE OnBufferStart(void* pBufferContext) override {}
    void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) override {
        static_cast<XAudio2SoundDriver::XAudio2SourceVoiceInstance*>(pBufferContext)
                ->on_buffer_end();
    }
    void STDMETHODCALLTYPE OnLoopEnd(void* pBufferContext) override {}
    void STDMETHODCALLTYPE OnVoiceError(void* pBufferContext, HRESULT Error) override {}
};

class XAudio2SoundDriver::XAudio2Channel : public SoundChannel {
  public:
    XAudio2Channel(XAudio2SoundDriver& driver)
            : _driver(driver),
              _source_voice(nullptr) {
        WAVEFORMATEX format;
        ZeroMemory(&format, sizeof(format));

        format.wFormatTag = WAVE_FORMAT_PCM;
        format.nChannels  = 2;
        format.nSamplesPerSec = _driver.get_sample_rate();
        format.wBitsPerSample = 16;
        format.nBlockAlign    = format.nChannels * format.wBitsPerSample / 8;
        format.nAvgBytesPerSec = format.nBlockAlign * format.nSamplesPerSec;

        XAUDIO2_SEND_DESCRIPTOR sendsList[1];
        sendsList[0].Flags = 0;
        sendsList[0].pOutputVoice = _driver.get_mv();

        XAUDIO2_VOICE_SENDS sends;
        sends.pSends = sendsList;
        sends.SendCount = sizeof(sendsList) / sizeof(sendsList[0]);

        IXAudio2SourceVoice* source_voice;
        check_hresult(
                "CreateSourceVoice", _driver.get_xa2()->CreateSourceVoice(
                                             &source_voice, &format, XAUDIO2_VOICE_NOSRC,
                                             XAUDIO2_MAX_FREQ_RATIO, _driver.get_voice_callbacks()));

        try {
            _source_voice = new XAudio2SourceVoiceInstance(_driver, source_voice);
        } catch (...) {
            source_voice->DestroyVoice();
            throw;
        }
    }

    ~XAudio2Channel() {
        if (_source_voice)
            _driver.garbage_collect(_source_voice);
    }

    void activate() override { _driver._active_channel = this; }

    void play(XAudio2Sound& sound, uint8_t volume) {
        if (!_source_voice)
            return;

        _source_voice->reset_and_play_sound(sound.sound_instance(), false);
    }

    void loop(XAudio2Sound& sound, uint8_t volume) {
        if (!_source_voice)
            return;

        _source_voice->reset_and_play_sound(sound.sound_instance(), true);
    }

    void quiet() override {
        if (!_source_voice)
            return;

        _source_voice->reset_and_play_sound(nullptr, false);
    }

  private:
    XAudio2SoundDriver&         _driver;
    XAudio2SourceVoiceInstance* _source_voice;
};

void XAudio2SoundDriver::XAudio2Sound::play(uint8_t volume) {
    _driver._active_channel->play(*this, volume);
}

void XAudio2SoundDriver::XAudio2Sound::loop(uint8_t volume) {
    _driver._active_channel->loop(*this, volume);
}

class XAudio2SoundDriver::XAudio2GarbageCollector {
  public:
    XAudio2GarbageCollector(XAudio2SoundDriver& driver)
            : _driver(driver),
              _wake_event(nullptr),
              _quit_event(nullptr),
              _idle_event(nullptr),
              _is_quitting(0),
              _next_collectable(nullptr) {
        _wake_event = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        _quit_event = CreateEventA(nullptr, FALSE, FALSE, nullptr);
        _idle_event = CreateEventA(nullptr, FALSE, FALSE, nullptr);

        if (_wake_event == nullptr || _quit_event == nullptr || _quit_event == nullptr)
            throw std::runtime_error("CreateEvent failed");

        if (!CreateThread(nullptr, 0, XAudio2GarbageCollector::thread_start, this, 0, nullptr))
            throw std::runtime_error("CreateThread failed");

        _is_started = true;
    }

    ~XAudio2GarbageCollector() {
        if (_is_started)
            this->shutdown();

        if (_quit_event)
            CloseHandle(_quit_event);

        if (_wake_event)
            CloseHandle(_wake_event);

        if (_idle_event)
            CloseHandle(_idle_event);
    }

    void link_object(XAudio2GarbageCollectable* obj) {
        // Put this at the head of the list
        for (;;) {
            XAudio2GarbageCollectable* next = _next_collectable;
            obj->set_next_gc(next);
            XAudio2GarbageCollectable* cas_result = cas_collectable(_next_collectable, next, obj);
            if (cas_result == next)
                break;
        }
    }

  private:
    void shutdown() {
        // Wait for any partial GC cycle to finish
        ResetEvent(_idle_event);
        SetEvent(_wake_event);
        WaitForSingleObject(_idle_event, INFINITE);

        // Run one more cycle
        SetEvent(_wake_event);
        WaitForSingleObject(_idle_event, INFINITE);

        // Quit
        _is_quitting = true;
        SetEvent(_wake_event);

        // Wait for the thread to quit
        WaitForSingleObject(_quit_event, INFINITE);
    }

    XAudio2GarbageCollectable* atomic_swap_chain_head(XAudio2GarbageCollectable* new_head) {
        return static_cast<XAudio2GarbageCollectable*>(InterlockedExchangePointer(
                reinterpret_cast<PVOID volatile*>(&_next_collectable), new_head));
    }

    static XAudio2GarbageCollectable* cas_collectable(
        XAudio2GarbageCollectable*volatile& dest, XAudio2GarbageCollectable* expected,
        XAudio2GarbageCollectable* replacement) {
        return static_cast<XAudio2GarbageCollectable*>(InterlockedCompareExchangePointer(
                reinterpret_cast<PVOID volatile*>(&dest), replacement, expected));
    }

    int thread_func() {
        for (;;) {
            WaitForSingleObject(_wake_event, INFINITE);

            XAudio2GarbageCollectable* next = atomic_swap_chain_head(nullptr);
            while (next != nullptr) {
                XAudio2GarbageCollectable* current = next;
                next = current->get_next_gc();

                delete current;
            }

            if (_is_quitting)
                break;

            SetEvent(_idle_event);
        }

        SetEvent(_quit_event);

        return 0;
    }

    static DWORD WINAPI thread_start(LPVOID thread_param) {
        return static_cast<DWORD>(
                static_cast<XAudio2GarbageCollector*>(thread_param)->thread_func());
    }

    XAudio2SoundDriver&                 _driver;
    HANDLE                              _wake_event;
    HANDLE                              _quit_event;
    HANDLE                              _idle_event;
    bool                                _is_started;
    XAudio2GarbageCollectable*          _next_collectable;

    bool                                _is_quitting;
};

XAudio2SoundDriver::XAudio2SoundDriver()
        : _xa2(nullptr),
          _mv(nullptr),
          _active_channel(nullptr),
          _sample_rate(44100),
          _is_com_initialized(false) {
    _gc = std::unique_ptr<XAudio2GarbageCollector>(new XAudio2GarbageCollector(*this));
    _voice_callbacks = std::unique_ptr<XAudio2VoiceCallbacks>(new XAudio2VoiceCallbacks());

    check_hresult("CoInitializeEx", CoInitializeEx(nullptr, COINIT_MULTITHREADED));
    _is_com_initialized = true;

    UINT      flags = 0;
#ifndef NDEBUG
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif

    _sample_rate = (_sample_rate + (XAUDIO2_QUANTUM_DENOMINATOR / 2)) /
                           XAUDIO2_QUANTUM_DENOMINATOR * XAUDIO2_QUANTUM_DENOMINATOR;

    check_hresult("XAudio2Create", XAudio2Create(&_xa2, flags, XAUDIO2_DEFAULT_PROCESSOR));
    check_hresult("CreateMasteringVoice", _xa2->CreateMasteringVoice(&_mv, 2, _sample_rate, 0, nullptr, nullptr, AudioCategory_GameEffects));
}

XAudio2SoundDriver::~XAudio2SoundDriver() {
    // Flush and clean up the garbage collector
    _gc.reset();

    if (_mv)
        _mv->DestroyVoice();

    if (_xa2)
        _xa2->Release();

    if (_is_com_initialized)
        CoUninitialize();
}

void XAudio2SoundDriver::garbage_collect(XAudio2GarbageCollectable* garbage_collectable) {
    _gc.get()->link_object(garbage_collectable);
}

unique_ptr<SoundChannel> XAudio2SoundDriver::open_channel() {
    return unique_ptr<SoundChannel>(new XAudio2Channel(*this));
}

unique_ptr<Sound> XAudio2SoundDriver::open_sound(pn::string_view path) {
    unique_ptr<XAudio2Sound> sound(new XAudio2Sound(*this));
    SoundData               s = Resource::sound(path);
    sound->buffer(s);
    return std::move(sound);
}

unique_ptr<Sound> XAudio2SoundDriver::open_music(pn::string_view path) {
    unique_ptr<XAudio2Sound> music(new XAudio2Sound(*this));
    SoundData               s = Resource::music(path);
    music->buffer(s);
    return std::move(music);
}

void XAudio2SoundDriver::set_global_volume(uint8_t volume) {
    if (_mv)
        _mv->SetVolume(static_cast<float>(volume) / 8.0f);
}

}  // namespace antares
