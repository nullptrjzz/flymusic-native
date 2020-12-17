// FlyAudio.cpp : 定义 DLL 的导出函数。
//

#pragma once
#ifdef _WIN32
#include "pch.h"
#include "framework.h"
#endif

#include "FlyAudio.h"
#include "bass.h"
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>
#include <taglib/tstring.h>
#include <vector>
#include <iostream>

#ifdef _WIN32
#pragma comment (lib, "bass.lib")
#pragma comment (lib, "tag.lib")
#pragma comment (lib, "tag_c.lib")
#else
#endif

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
		BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, true);
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
		unsigned long long len = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
		return BASS_ChannelBytes2Seconds(stream, len);
	}

	FLYAUDIO_API double getPosition() {
		unsigned long long len = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
		return BASS_ChannelBytes2Seconds(stream, len);
	}

	FLYAUDIO_API unsigned long long getDurationB() {
		unsigned long long len = BASS_ChannelGetLength(stream, BASS_POS_BYTE);
		return len;
	}

	FLYAUDIO_API unsigned long long getPositionB() {
		unsigned long long len = BASS_ChannelGetPosition(stream, BASS_POS_BYTE);
		return len;
	}

	FLYAUDIO_API void setPosition(double sec) {
		unsigned long long pos = BASS_ChannelSeconds2Bytes(stream, sec);
		BASS_ChannelSetPosition(stream, pos, BASS_POS_BYTE);
	}

	FLYAUDIO_API void setPositionB(unsigned long long pos) {
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

	FLYAUDIO_API AUDIO_META* audioMeta(const char* file) {
		TagLib::FileRef f(file);
		AUDIO_META* meta = new AUDIO_META();
		TagLib::Tag* tag = f.tag();
		meta->bitRate = f.audioProperties()->bitrate();
		meta->channels = f.audioProperties()->channels();
		meta->length = f.audioProperties()->length();
		meta->sampleRate = f.audioProperties()->sampleRate();
		
		meta->title = tag->title().toCString(true);
		meta->artist = tag->artist().toCString(true);
		meta->album = tag->album().toCString(true);
		meta->comment = tag->comment().toCString(true);
		meta->genre = tag->genre().toCString(true);
		meta->year = tag->year();
		meta->track = tag->track();

		const char** otherProps = 
			new const char* []{"ALBUMARTIST", "SUBTITLE", "DISCNUMBER", "DATE", "ORIGINALDATE", "COMPOSER", "LYRICIST", "CONDUCTOR", "REMIXER", "PERFORMER"};
		TagLib::PropertyMap map = tag->properties();
		
		
	
		free(tag);
		return meta;
	}
}