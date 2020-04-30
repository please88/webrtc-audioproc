WebRTC AudioProc (AEC, VAD, NS...)


- 编译

	./autogen.sh
	./configure
	make & make install
	
- 测试程序

	cd /home/webrtc-audioproc/modules/audio_processing/test
	make -f makefile_ns
	./ns_test mix_apm_far.pcm 

