#pragma once

class Sound
{
public:
	static bool Init(int channels, bool enableFlag);
	static void Enable(bool enableFlag);
	static void Activate();
	static void Deactivate();
	static void Close();
	static void PlaySound(AudioClip* wavePtr, int time);
	static AudioClip* LoadWaveFile(const std::string& lpName);
	static void FreeSound(AudioClip* wave);
	static void SetChannels(int channels);
private:
	static int num_channels;
	static bool enabled_flag;
	static int* TimeStamps;
	static AudioVoice* Voices;
};
