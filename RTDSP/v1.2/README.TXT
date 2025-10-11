WAVE Audio Real Time Delay DSP for Windows
Version 1.2

This application supports .wav files, 16bit and 24bit.

This audio effect is the same as the GNU-Linux_AudioDelay project (https://github.com/RMSabe/GNU-Linux_AudioDelay). Same logic, same controls, pretty much same code, but for Windows.

Unfortunately, I still didn't figure out completely how Windows MMDeviceAPI works, so there are still a lot of limitations to this application:

1. It will only access the playback audio device in exclusive mode.
(I tried using shared mode, but that doesn't use either WAVE or IEEE FLOAT streaming format, instead it uses another format that I don't know how works).
This means that if any other application is using the intended audio device, this application will not work.
If this application is running, no other application will have access to the audio device.

2. Since the application accesses the audio device in exclusive mode, bit-depth, sample-rate and number of channels must be compatible with the audio device for it to work.

3. For this application, I'm focusing more on mono and stereo audio files. Files with more channels might work, but channels might be misplaced.
I do not recommend using this application for audio files with more than 2 channels.

Latest Update:
Native support for 24bit audio. 
Some bug fixes.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
