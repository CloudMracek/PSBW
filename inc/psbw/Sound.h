#pragma once

void spu_init();


/// @brief Plays an audio track from the CDROM
void sound_play_cdda(int track, int loop);

/// @brief Stops CDROM audio playback
void sound_stop_cdda();

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

        /**
         * \brief Play's the selected file. Loops if Loop flag is set in VAG file
        */
        void play();

        
    private:
        int soundAddr;
        int sampleRate;
        void spu_upload_sample(const void *data);
        
};