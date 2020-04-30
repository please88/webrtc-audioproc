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
