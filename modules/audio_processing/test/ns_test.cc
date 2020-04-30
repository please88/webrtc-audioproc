

#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>


#include "audio_processing.h"
#include "module_common_types.h"

using webrtc::AudioFrame;
using webrtc::AudioProcessing;
using webrtc::EchoCancellation;
using webrtc::GainControl;
using webrtc::NoiseSuppression;


int main(int argc, char* argv[]){

    char near_filename[50];
    char out_filename[50];

    int32_t sample_rate_hz = 8000;
    int32_t device_sample_rate_hz = 8000;

    int num_capture_input_channels = 1;
    int num_capture_output_channels = 1;
    int num_render_channels = 1;

    int samples_per_channel = sample_rate_hz / 100;
    char slience[AudioFrame::kMaxDataSizeSamples] = {0};

    if(argc != 2){
        printf("please input pcm file\n");
        return -1;
    }

    sprintf(near_filename, "%s", argv[1]);
    sprintf(out_filename, "ns_out_%s", argv[1]);

    FILE* near_file = NULL;
    FILE* out_file = NULL;

    size_t read_count = 0;
    int primary_count = 0;
    int near_read_bytes = 0;

    AudioFrame near_frame;

    int8_t stream_has_voice = 0;
    float ns_speech_prob = 0.0f;

    near_file = fopen(near_filename, "rb");
    out_file = fopen(out_filename, "wb");

    int near_size_bytes = 0;
    struct stat st;
    stat(near_filename, &st);
    near_size_bytes = st.st_size;

    AudioProcessing* apm = AudioProcessing::Create(0);
    //apm->level_estimator()->Enable(true);
    apm->set_sample_rate_hz(sample_rate_hz);
    apm->set_num_channels(num_capture_input_channels,
                        num_capture_output_channels);
    apm->set_num_reverse_channels(num_render_channels);


    //apm->gain_control()->Enable(true);
    //if(apm->gain_control()->is_enabled())
    //    printf("gain_control is_enabled\n");
        
    apm->noise_suppression()->Enable(true);
    if(apm->noise_suppression()->is_enabled())
        printf("noise_suppression is_enabled\n");

    int8_t enable_vad = false;
    if(enable_vad){
        apm->voice_detection()->Enable(true);
        if(apm->voice_detection()->is_enabled())
            printf("voice_detection is_enabled\n");
    }

    while (feof(near_file) == 0) 
    {

        primary_count++;
        near_frame.num_channels_ = num_capture_input_channels;
        near_frame.sample_rate_hz_ = sample_rate_hz;
        near_frame.samples_per_channel_ = samples_per_channel;

        size_t size = samples_per_channel * num_capture_input_channels;
        read_count = fread(near_frame.data_,
                           sizeof(int16_t),
                           size,
                           near_file);
        if(read_count != size)
        {
            break;
        }

        near_read_bytes += read_count * sizeof(int16_t);
        if (primary_count % 100 == 0) {
          printf("%.0f%% complete\n",
              (near_read_bytes * 100.0) / near_size_bytes);
        }


        int err = apm->ProcessStream(&near_frame);
        if (err != AudioProcessing::kNoError) {
            printf("ProcessStream error %d\n", err);
        }

        stream_has_voice =
            static_cast<int8_t>(apm->voice_detection()->stream_has_voice());

        //printf("%d\n", stream_has_voice);
        size = samples_per_channel * near_frame.num_channels_;
        if (stream_has_voice || !enable_vad) {
            fwrite(near_frame.data_,
                                   sizeof(int16_t),
                                   size,
                                   out_file);
        }
        else{
            fwrite(slience,
                                   sizeof(int16_t),
                                   size,
                                   out_file);
        }
    }

    AudioProcessing::Destroy(apm);
    apm = NULL;

    return 0;

}


