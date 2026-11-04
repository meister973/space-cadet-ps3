#include <string.h>

#include "check.h"
#include "wav.h"

static unsigned char file[256];
static size_t file_size;

static void put_le16(unsigned value) { file[file_size++] = value & 0xFF; file[file_size++] = value >> 8; }
static void put_le32(unsigned long value) { put_le16(value & 0xFFFF); put_le16(value >> 16); }
static void put_tag(const char *tag) { memcpy(file + file_size, tag, 4); file_size += 4; }

/* A RIFF header and fmt chunk; the caller appends the remaining chunks. */
static void begin_wav(unsigned format, unsigned channels, unsigned long rate, unsigned bits)
{
    file_size = 0;
    put_tag("RIFF");
    put_le32(0);
    put_tag("WAVE");
    put_tag("fmt ");
    put_le32(16);
    put_le16(format);
    put_le16(channels);
    put_le32(rate);
    put_le32(rate * channels * bits / 8);
    put_le16(channels * bits / 8);
    put_le16(bits);
}

static void put_data(const unsigned char *samples, size_t count, unsigned long declared_size)
{
    put_tag("data");
    put_le32(declared_size);
    memcpy(file + file_size, samples, count);
    file_size += count;
}

static void eight_bit_mono_plays_on_both_sides_centered_on_128(void)
{
    const unsigned char samples[] = { 128, 255, 0 };
    WavSound sound;

    begin_wav(1, 1, 11025, 8);
    put_data(samples, sizeof(samples), sizeof(samples));

    CHECK(wav_decode(file, file_size, &sound) == WAV_OK);
    CHECK(sound.frame_count == 3);
    CHECK(sound.rate == 11025);
    CHECK_NEAR(sound.frames[0], 0.0, 1e-6);
    CHECK_NEAR(sound.frames[1], 0.0, 1e-6);
    CHECK_NEAR(sound.frames[2], 127.0 / 128.0, 1e-6);
    CHECK_NEAR(sound.frames[3], 127.0 / 128.0, 1e-6);
    CHECK_NEAR(sound.frames[4], -1.0, 1e-6);
    CHECK_NEAR(sound.frames[5], -1.0, 1e-6);
    wav_free(&sound);
}

static void sixteen_bit_stereo_reads_little_endian_signed_samples(void)
{
    const unsigned char samples[] = { 0x00, 0x40, 0x00, 0xC0 };
    WavSound sound;

    begin_wav(1, 2, 22050, 16);
    put_data(samples, sizeof(samples), sizeof(samples));

    CHECK(wav_decode(file, file_size, &sound) == WAV_OK);
    CHECK(sound.frame_count == 1);
    CHECK_NEAR(sound.frames[0], 0.5, 1e-6);
    CHECK_NEAR(sound.frames[1], -0.5, 1e-6);
    wav_free(&sound);
}

static void chunks_before_data_are_skipped(void)
{
    const unsigned char samples[] = { 255 };
    WavSound sound;

    begin_wav(1, 1, 11025, 8);
    put_tag("LIST");
    put_le32(3);
    put_tag("abc");      /* three bytes of payload, then one pad byte for even alignment */
    put_data(samples, sizeof(samples), sizeof(samples));

    CHECK(wav_decode(file, file_size, &sound) == WAV_OK);
    CHECK(sound.frame_count == 1);
    CHECK_NEAR(sound.frames[0], 127.0 / 128.0, 1e-6);
    wav_free(&sound);
}

static void a_data_chunk_longer_than_the_file_plays_the_bytes_present(void)
{
    const unsigned char samples[] = { 0, 0 };
    WavSound sound;

    begin_wav(1, 1, 11025, 8);
    put_data(samples, sizeof(samples), 1000);

    CHECK(wav_decode(file, file_size, &sound) == WAV_OK);
    CHECK(sound.frame_count == 2);
    wav_free(&sound);
}

static void a_file_that_is_not_riff_is_rejected(void)
{
    WavSound sound;

    begin_wav(1, 1, 11025, 8);
    memcpy(file, "RIFX", 4);

    CHECK(wav_decode(file, file_size, &sound) == WAV_NOT_RIFF);
}

static void compressed_sound_is_rejected(void)
{
    const unsigned char samples[] = { 0 };
    WavSound sound;

    begin_wav(2, 1, 11025, 4);
    put_data(samples, sizeof(samples), sizeof(samples));

    CHECK(wav_decode(file, file_size, &sound) == WAV_UNSUPPORTED);
}

static void a_file_without_data_is_truncated(void)
{
    WavSound sound;

    begin_wav(1, 1, 11025, 8);

    CHECK(wav_decode(file, file_size, &sound) == WAV_TRUNCATED);
}

int main(void)
{
    RUN(eight_bit_mono_plays_on_both_sides_centered_on_128);
    RUN(sixteen_bit_stereo_reads_little_endian_signed_samples);
    RUN(chunks_before_data_are_skipped);
    RUN(a_data_chunk_longer_than_the_file_plays_the_bytes_present);
    RUN(a_file_that_is_not_riff_is_rejected);
    RUN(compressed_sound_is_rejected);
    RUN(a_file_without_data_is_truncated);
    return 0;
}
