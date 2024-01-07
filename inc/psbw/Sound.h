#pragma once

void spu_init();
void spu_play_sample(int addr, int sample_rate);

class Sound {
    public:
        Sound(const void *data);
        void play();
    private:
        int soundAddr;
        int sampleRate;
        void spu_upload_sample(const void *data);
        
};