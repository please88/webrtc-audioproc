WebRTC AudioProc (AEC, VAD, NS...)


- 编译

<pre>
	./autogen.sh
	./configure
	make & make install
</pre>

- 测试程序

<pre>
	cd /home/webrtc-audioproc/modules/audio_processing/test
	make -f makefile_ns
	./ns_test mix_apm_far.pcm
</pre>


- ffmpeg命令将mp3转成pcm

<pre>
	单通道，采样率8000, s16le PCM signed 16-bit little-endian
	D:\ffmpeg-4.2.2-win64-static\bin>
	ffmpeg.exe -i "D:\视频会议\3.mp3" -f s16le -ar 8000 -ac 1 -acodec pcm_s16le "D:\视频会议\3_16k.pcm"

	查看支持的格式
	ffmpeg.exe -formats 
</pre>

