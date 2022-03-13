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

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN 1
#endif

#ifndef NOMINMAX
#  define NOMINMAX 1
#endif

#include <windows.h>

#include <xaudio2.h>

using std::unique_ptr;

namespace antares {

namespace {

std::vector<BYTE> convert_to_stereo(const BYTE* input_data, size_t num_channels, size_t num_samples) {
    std::vector<BYTE> output_data;
    output_data.resize(num_samples * 2 * sizeof(int16_t));

    int16_t*       output_data_i16 = reinterpret_cast<int16_t*>(output_data.data());
    const int16_t* input_data_i16  = reinterpret_cast<const int16_t*>(input_data);

    if (num_channels == 1) {
        for (size_t i = 0; i < num_samples; i++) {
            output_data_i16[i * 2 + 0] = output_data_i16[i * 2 + 1] = input_data_i16[i];
        }
    } else if (num_channels == 2) {
        memcpy(output_data_i16, input_data_i16, num_samples * 2 * sizeof(int16_t));
    } else {
        for (size_t i = 0; i < num_samples; i++) {
            output_data_i16[i * 2 + 0] = input_data_i16[i * num_channels + 0];
            output_data_i16[i * 2 + 1] = input_data_i16[i * num_channels + 1];
        }
    }
    return std::move(output_data);
}

void check_hresult(pn::string_view method, HRESULT hresult) {
    if (FAILED(hresult)) {
        throw std::runtime_error(
                pn::format("{0}: {1}", method, static_cast<int>(hresult)).c_str());
    }
}

}  // namespace

class XAudio2SoundDriver::XAudio2SoundInstance {
  public:
    XAudio2SoundInstance(
            XAudio2SoundDriver& driver, std::vector<BYTE>&& data, float frequency_ratio)
            : _driver(driver),
              _data(std::move(data)),
              _frequency_ratio(frequency_ratio)
    {}

    const BYTE* get_data() const { return _data.data(); }
    size_t      get_data_size() const { return _data.size(); }
    float       get_frequency_ratio() const { return _frequency_ratio; }

  private:
    XAudio2SoundDriver&     _driver;
    float                   _frequency_ratio;
    std::vector<BYTE>       _data;
};

class XAudio2SoundDriver::XAudio2VoiceInstance {
  public:
    XAudio2VoiceInstance(XAudio2SoundDriver& driver, IXAudio2Voice* voice)
            : _driver(driver), _voice(voice) {}

    ~XAudio2VoiceInstance() { _voice->DestroyVoice(); }

  protected:
    XAudio2SoundDriver& _driver;
    IXAudio2Voice*      _voice;
};

class XAudio2SoundDriver::XAudio2SourceVoiceInstance {
  public:
    XAudio2SourceVoiceInstance(XAudio2SoundDriver& driver, IXAudio2SourceVoice* voice)
            : _driver(driver),
              _voice(voice),
              _active_sound(nullptr),
              _active_is_loop(false),
              _active_volume(1.0f),
              _pending_sound(nullptr),
              _pending_looping(false),
              _pending_volume(1.0f),
              _source_voice_state(SourceVoiceState::Idle)
    {
        InitializeCriticalSection(&_mutex);
    }

    ~XAudio2SourceVoiceInstance() {
        reset_and_play_sound(nullptr, false, 0.0f);

        _voice->DestroyVoice();

        DeleteCriticalSection(&_mutex);

        assert(_active_sound == nullptr);
    }

    IXAudio2SourceVoice* get_voice() const { _voice; }

    void on_buffer_end();
    void reset_and_play_sound(const std::shared_ptr<XAudio2SoundInstance>& sound, bool looping, float volume);

  private:
    // To simplify XAudio2 voice state handling, we only ever submit one buffer at a time.
    enum class SourceVoiceState {
        Idle,                     // No buffers are submitted, _active_sound and _pending_sound are both null.
        Playing,                  // A buffer was submitted and will eventually play.
        Resetting,                // A buffer was submitted, but a stop was also submitted.  OnBufferEnd will transition to pending state.
    };

    void submit_active_sound();
    void set_and_submit_sound(const std::shared_ptr<XAudio2SoundInstance>& sound, bool looping, float volume);

    XAudio2SoundDriver&   _driver;

    IXAudio2SourceVoice*  _voice;

    // Mutex-guarded fields
    CRITICAL_SECTION                        _mutex;
    std::shared_ptr<XAudio2SoundInstance>   _pending_sound;
    float                                   _pending_volume;
    bool                                    _pending_looping;

    std::shared_ptr<XAudio2SoundInstance>   _active_sound;
    float                                   _active_volume;
    bool                                    _active_is_loop;

    SourceVoiceState                        _source_voice_state;
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
        _active_sound       = _pending_sound;
        _active_is_loop     = _pending_looping;
        _active_volume      = _pending_volume;

        _pending_sound = nullptr;
        _pending_looping = false;
        _pending_volume  = 1.0f;

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
        const std::shared_ptr<XAudio2SoundInstance>& sound, bool looping, float volume) {

    EnterCriticalSection(&_mutex);

    switch (_source_voice_state) {
        case SourceVoiceState::Idle:
            // No sound is submitted, submit this one
            assert(!_pending_sound);
            assert(!_active_sound);
            if (sound) {
                _source_voice_state = SourceVoiceState::Playing;
                set_and_submit_sound(sound, looping, volume);
            }
        break;
        case SourceVoiceState::Playing:
            // A sound was submitted, stop it and change to the specified one
            _pending_sound      = sound;
            _source_voice_state = SourceVoiceState::Resetting;
            _voice->Stop(0, 0);
            _voice->FlushSourceBuffers();
            break;
        case SourceVoiceState::Resetting:
            // A sound was submitted but a stop was already requested, wait for it
            _pending_sound = sound;
            break;
        default: break;
    }
    _pending_looping = looping;
    _pending_volume  = volume;

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

    UINT32 operation_set = _driver.alloc_operation_set();

    _voice->SetFrequencyRatio(_active_sound->get_frequency_ratio(), operation_set);
    _voice->SetVolume(_active_volume, operation_set);
    _voice->Start(0, operation_set);

    _driver.get_xa2()->CommitChanges(operation_set);

    if (FAILED(_voice->SubmitSourceBuffer(&buffer, nullptr))) {
        _active_sound       = nullptr;
        _source_voice_state = SourceVoiceState::Idle;
    }
}

void XAudio2SoundDriver::XAudio2SourceVoiceInstance::set_and_submit_sound(
    const std::shared_ptr<XAudio2SoundInstance>& sound, bool looping, float volume) {
    _active_sound   = sound;
    _active_is_loop = looping;
    _active_volume  = volume;
    submit_active_sound();
}

class XAudio2SoundDriver::XAudio2Sound : public Sound {
  public:
    XAudio2Sound(XAudio2SoundDriver& driver)
            : _driver(driver), _instance(nullptr) {}

    virtual void play(uint8_t volume);
    virtual void loop(uint8_t volume);

    void buffer(const SoundData& s) {
        _instance.reset();

        if (s.channels == 0) {
            return;
        }

        const size_t num_samples = s.data.size() / s.channels / sizeof(int16_t);
        std::vector<BYTE> data = convert_to_stereo(
                reinterpret_cast<const BYTE*>(s.data.data()), s.channels, num_samples);

        float  frequency_ratio =
                static_cast<float>(s.frequency) / static_cast<float>(_driver.get_sample_rate());

        frequency_ratio = std::min(
                XAUDIO2_MAX_FREQ_RATIO, std::max(XAUDIO2_MIN_FREQ_RATIO, frequency_ratio));

        _instance = std::make_shared<XAudio2SoundInstance>(_driver, std::move(data), frequency_ratio);
    }

    const std::shared_ptr<XAudio2SoundInstance>& sound_instance() const { return _instance; }

  private:
    XAudio2SoundDriver&                   _driver;
    std::shared_ptr<XAudio2SoundInstance> _instance;
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
                        &source_voice, &format, 0, XAUDIO2_MAX_FREQ_RATIO, _driver.get_voice_callbacks()));

        try {
            _source_voice.reset(new XAudio2SourceVoiceInstance(_driver, source_voice));
        } catch (...) {
            source_voice->DestroyVoice();
            throw;
        }
    }

    void activate() override { _driver._active_channel = this; }

    void play(XAudio2Sound& sound, uint8_t volume) {
        _source_voice->reset_and_play_sound(sound.sound_instance(), false, static_cast<float>(volume) / 255.0f);
    }

    void loop(XAudio2Sound& sound, uint8_t volume) {
        _source_voice->reset_and_play_sound(
                sound.sound_instance(), true, static_cast<float>(volume) / 255.0f);
    }

    void quiet() override {
        _source_voice->reset_and_play_sound(nullptr, false, 0.0f);
    }

  private:
    XAudio2SoundDriver&         _driver;
    std::unique_ptr<XAudio2SourceVoiceInstance> _source_voice;
};

void XAudio2SoundDriver::XAudio2Sound::play(uint8_t volume) {
    _driver._active_channel->play(*this, volume);
}

void XAudio2SoundDriver::XAudio2Sound::loop(uint8_t volume) {
    _driver._active_channel->loop(*this, volume);
}

XAudio2SoundDriver::XAudio2SoundDriver()
        : _xa2(nullptr),
          _mv(nullptr),
          _active_channel(nullptr),
          _sample_rate(44100),
          _is_com_initialized(false),
          _next_operation_set(1) {
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
    if (_mv)
        _mv->DestroyVoice();

    if (_xa2)
        _xa2->Release();

    if (_is_com_initialized)
        CoUninitialize();
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

uint32_t XAudio2SoundDriver::alloc_operation_set() {
    return static_cast<uint32_t>(InterlockedIncrement(&_next_operation_set) - 1);
}

}  // namespace antares
