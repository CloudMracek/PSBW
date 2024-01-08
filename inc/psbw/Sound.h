#pragma once

void spu_init();
void spu_play_sample(int addr, int sample_rate);

class Sound {
    public:

        // Uploads specified VAG data into SPU RAM. VAGi not supported.
        Sound(const void *data);

        // Plays the selected file. Loops if Loop flag is set in VAG file
        void play();

        
    private:
        int soundAddr;
        int sampleRate;
        void spu_upload_sample(const void *data);
        
};