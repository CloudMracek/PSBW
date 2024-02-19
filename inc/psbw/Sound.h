#pragma once

#include <stddef.h>

void spu_init();

void spu_upload(const void *data, size_t size);

/// @brief Plays an audio track from the CDROM
void soundPlayCdda(int track, int loop);

/// @brief Stops CDROM audio playback
void soundStopCdda();

void spu_play_sample(int addr, int sample_rate);

/**
 * \class Sound
 * \brief A single sound file whhich can be played
 *
 *
 */
class Sound {
    public:

        /**
         * \brief Uploads specified VAG data into SPU RAM. VAGi not supported.
        */
        Sound(const void *data);
        Sound();

        /**
         * \brief Play's the selected file. Loops if Loop flag is set in VAG file
        */
        void play();

        int soundAddr;
        int sampleRate;
    private:
        void spu_upload_sample(const void *data);
        
};