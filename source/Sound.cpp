#include "pch.h"
#include "Sound.h"

#include <memory>

#include "channels.h"

namespace
{
	// The original game mixes at full volume; this leaves headroom for several effects at once.
	constexpr float EffectVolume = 0.8f;
}

int Sound::num_channels;
bool Sound::enabled_flag = false;
int* Sound::TimeStamps = nullptr;
AudioVoice* Sound::Voices = nullptr;

bool Sound::Init(int channels, bool enableFlag)
{
	auto result = audio_open();
	SetChannels(channels);
	Enable(enableFlag);
	return result == 0;
}

void Sound::Enable(bool enableFlag)
{
	enabled_flag = enableFlag;
	if (!enableFlag)
		audio_stop_all();
}

void Sound::Activate()
{
	audio_set_paused(false);
}

void Sound::Deactivate()
{
	audio_set_paused(true);
}

void Sound::Close()
{
	audio_close();
	delete[] TimeStamps;
	delete[] Voices;
	TimeStamps = nullptr;
	Voices = nullptr;
}

// A limited number of channels, as in the original: when all are busy, the oldest sound stops.
void Sound::PlaySound(AudioClip* wavePtr, int time)
{
	if (!wavePtr || !enabled_flag)
		return;

	auto busy = std::make_unique<bool[]>(num_channels);
	for (auto channel = 0; channel < num_channels; channel++)
		busy[channel] = audio_voice_is_playing(Voices[channel]);

	auto channel = channels_pick(TimeStamps, busy.get(), num_channels);
	audio_voice_stop(Voices[channel]);
	Voices[channel] = audio_voice_start(wavePtr, EffectVolume, false);
	TimeStamps[channel] = time;
}

AudioClip* Sound::LoadWaveFile(const std::string& lpName)
{
	return audio_clip_load_wav(lpName.c_str());
}

// Clips live until the process exits: the mixer thread may still be reading one.
void Sound::FreeSound(AudioClip* wave)
{
	(void)wave;
}

void Sound::SetChannels(int channels)
{
	if (channels <= 0)
		channels = 8;

	num_channels = channels;
	delete[] TimeStamps;
	delete[] Voices;
	TimeStamps = new int[num_channels]();
	Voices = new AudioVoice[num_channels];
	std::fill(Voices, Voices + num_channels, AUDIO_NO_VOICE);
}
