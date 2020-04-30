

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


int main(int argc, char* argv[])
{

	const char* pb_filename = NULL;
	const char* far_filename = NULL;
	const char* near_filename = NULL;
	const char* out_filename = NULL;
	const char* vad_out_filename = NULL;
	const char* ns_prob_filename = NULL;
	const char* aecm_echo_path_in_filename = NULL;
	const char* aecm_echo_path_out_filename = NULL;
	
	int32_t sample_rate_hz = 8000;
	int32_t device_sample_rate_hz = 8000;
	
	int num_capture_input_channels = 1;
	int num_capture_output_channels = 1;
	int num_render_channels = 1;
	
	int samples_per_channel = sample_rate_hz / 100;
	int extra_delay_ms = 0;

	const char far_file_default[] = "apm_far.pcm"; //"speaker.pcm";//"apm_far.pcm";
	const char near_file_default[] = "apm_near.pcm"; //"micin.pcm";//"apm_near.pcm";
	const char out_file_default[] = "out.pcm";
	const char event_filename[] = "apm_event.dat";
	const char delay_filename[] = "apm_delay.dat";
	const char drift_filename[] = "apm_drift.dat";
	const char vad_file_default[] = "vad_out.dat";
	const char ns_prob_file_default[] = "ns_prob.dat";

	far_filename = far_file_default;
	near_filename = near_file_default;
	out_filename = out_file_default;
	vad_out_filename = vad_file_default;
	ns_prob_filename = ns_prob_file_default;
	
	FILE* pb_file = NULL;
	FILE* far_file = NULL;
	FILE* near_file = NULL;
	FILE* out_file = NULL;
	FILE* event_file = NULL;
	FILE* delay_file = NULL;
	FILE* drift_file = NULL;
	FILE* vad_out_file = NULL;
	FILE* ns_prob_file = NULL;
	FILE* aecm_echo_path_in_file = NULL;
	FILE* aecm_echo_path_out_file = NULL;


	size_t read_count = 0;
	int reverse_count = 0;
	int primary_count = 0;
	int near_read_bytes = 0;
	
	AudioFrame far_frame;
	AudioFrame near_frame;
	
	int delay_ms = 0;
	int drift_samples = 0;
	int capture_level = 127;
	int8_t stream_has_voice = 0;
	float ns_speech_prob = 0.0f;
	
	far_file = fopen(far_filename, "rb");
	near_file = fopen(near_filename, "rb");
	out_file = fopen(out_filename, "wb");
	vad_out_file = fopen(vad_out_filename, "wb");
    ns_prob_file = fopen(ns_prob_filename, "wb");

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

	apm->echo_cancellation()->Enable(true);
	if(apm->echo_cancellation()->is_enabled())
		printf("echo_cancellation is_enabled\n");
	
	//apm->echo_cancellation()->enable_metrics(true);
	//apm->echo_cancellation()->enable_delay_logging(true);
	//apm->echo_cancellation()->enable_drift_compensation(true);
	//apm->echo_cancellation()->set_device_sample_rate_hz(device_sample_rate_hz);
	
	apm->gain_control()->Enable(true);
	if(apm->gain_control()->is_enabled())
		printf("gain_control is_enabled\n");
		
	apm->noise_suppression()->Enable(true);
	if(apm->noise_suppression()->is_enabled())
		printf("noise_suppression is_enabled\n");
	
	apm->voice_detection()->Enable(true);
	if(apm->voice_detection()->is_enabled())
		printf("voice_detection is_enabled\n");


	while (feof(far_file) == 0 && reverse_count < 155)
	{
		far_frame.sample_rate_hz_ = sample_rate_hz;
		far_frame.samples_per_channel_ = samples_per_channel;
		far_frame.num_channels_ = num_render_channels;		
		size_t size = samples_per_channel * num_render_channels;
		read_count = fread(far_frame.data_,
						   sizeof(int16_t),
						   size,
						   far_file);
		if(read_count != size)
		{
			break;
		}
		
		reverse_count++;
		apm->AnalyzeReverseStream(&far_frame);
	}

	{

		while (feof(far_file) == 0) {

			far_frame.sample_rate_hz_ = sample_rate_hz;
			far_frame.samples_per_channel_ = samples_per_channel;
			far_frame.num_channels_ = num_render_channels;
			near_frame.sample_rate_hz_ = sample_rate_hz;
			near_frame.samples_per_channel_ = samples_per_channel;

			size_t size = samples_per_channel * num_render_channels;
			read_count = fread(far_frame.data_,
							   sizeof(int16_t),
							   size,
							   far_file);
			if(read_count != size)
			{
				break;
			}
		
			reverse_count++;
			apm->AnalyzeReverseStream(&far_frame);
			

		  
		  //else if (event == kCaptureEvent) 
		  {
			primary_count++;
			near_frame.num_channels_ = num_capture_input_channels;
	
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

			delay_ms = 60;
			drift_samples = 0;

			//int capture_level_in = capture_level;
			//apm->gain_control()->set_stream_analog_level(capture_level);
			apm->set_stream_delay_ms(delay_ms + extra_delay_ms);
			//apm->echo_cancellation()->set_stream_drift_samples(drift_samples);

			int err = apm->ProcessStream(&near_frame);
			if (err == apm->kBadStreamParameterWarning) {
			  printf("Bad parameter warning.\n");
			}

			//printf("stream_has_echo %d\n", 
			//		apm->echo_cancellation()->stream_has_echo());
			
			//capture_level = apm->gain_control()->stream_analog_level();
			//stream_has_voice =
			//	static_cast<int8_t>(apm->voice_detection()->stream_has_voice());
			if (vad_out_file != NULL) {
				fwrite(&stream_has_voice,
								   sizeof(stream_has_voice),
								   1,
								   vad_out_file);
			}
	
			//if (ns_prob_file != NULL) {
			//  ns_speech_prob = apm->noise_suppression()->speech_probability();
			//	fwrite(&ns_speech_prob,
			//					   sizeof(ns_speech_prob),
			//					   1,
			//					   ns_prob_file);
			//}
	
	
			size = samples_per_channel * near_frame.num_channels_;
			fwrite(near_frame.data_,
								   sizeof(int16_t),
								   size,
								   out_file);
		  }

		}
 	}

	AudioProcessing::Destroy(apm);
	apm = NULL;

	return 0;
}






