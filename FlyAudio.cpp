// FlyAudio.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "FlyAudio.h"
#include "bass.h"
#include <vector>
#include <iostream>
#pragma comment (lib, "bass.lib")

extern "C" {
	int deviceCount = 0;
	HSTREAM stream = 0;

	FLYAUDIO_API void init(int device, int freq) {
		// BASS_Init(-1, 44100, 0, 0, 0);
		BASS_Init(device, freq, 0, 0, 0);
	}

	FLYAUDIO_API DEVICE_INFO** getDevices() {
		int a;
		deviceCount = 0;
		std::vector<DEVICE_INFO*> vec;
		BASS_DEVICEINFO info;
		for (a = 1; BASS_GetDeviceInfo(a, &info); a++) {
			if (info.flags & BASS_DEVICE_ENABLED) {
				deviceCount++;
				DEVICE_INFO *dinfo = new DEVICE_INFO();
				dinfo->driver = info.driver;
				dinfo->name = info.name;
				dinfo->flags = info.flags;
				dinfo->isDefault = (info.flags & BASS_DEVICE_DEFAULT) == 0 ? 0 : 1;
				vec.push_back(dinfo);
			}
		}

		DEVICE_INFO** ret = new DEVICE_INFO*[deviceCount];
		for (int i = 0; i < deviceCount; i++) {
			ret[i] = vec[i];
		}
		return ret;
	}

	FLYAUDIO_API int getDeviceCount() {
		return deviceCount;
	}

	FLYAUDIO_API bool setDevice(int id) {
		return BASS_SetDevice(id);
	}

	FLYAUDIO_API int loadFile(const char* file) {
		stream = BASS_StreamCreateFile(FALSE, file, 0, 0, 0);
		if (stream != 0) {
			return 0;
		} else {
			return BASS_ErrorGetCode();
		}
	}

	FLYAUDIO_API bool play() {
		return BASS_ChannelPlay(stream, false);
	}

	FLYAUDIO_API bool pause() {
		return BASS_ChannelPause(stream);
	}

	FLYAUDIO_API bool stop() {
		return BASS_ChannelStop(stream);
	}

	FLYAUDIO_API double getDuration() {
		QWORD len = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
		return BASS_ChannelBytes2Seconds(stream, len);
	}

	FLYAUDIO_API double getPosition() {
		QWORD len = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
		return BASS_ChannelBytes2Seconds(stream, len);
	}

	FLYAUDIO_API void setPosition(double sec) {
		QWORD pos = BASS_ChannelSeconds2Bytes(stream, sec);
		BASS_ChannelSetPosition(stream, pos, BASS_POS_BYTE);
	}

	FLYAUDIO_API int setVolume(int vol) {
		float _vol = (float) (vol < 0 ? 0 : (vol > 100 ? 100 : vol)) / (float) 100.0;
		BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, _vol);
		return getVolume();
	}

	FLYAUDIO_API int getVolume() {
		float vol;
		BASS_ChannelGetAttribute(stream, BASS_ATTRIB_VOL, &vol);
		return (int) (100.0 * vol);
	}

	FLYAUDIO_API float getCpu() {
		return BASS_GetCPU();
	}

	FLYAUDIO_API void freeStream() {
		BASS_StreamFree(stream);
		stream = 0;
	}

	FLYAUDIO_API void close() {
		BASS_StreamFree(stream);
		BASS_Free();
	}
}
