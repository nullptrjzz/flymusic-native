// FlyAudio.cpp : 定义 DLL 的导出函数。
//
#pragma warning(disable:4996)
#include <vector>
#include <iostream>

#include "FlyAudio.h"
#include "bass.h"
#include "json.hpp"
#include "md5.h"
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/tmap.h>
#include <taglib/tpropertymap.h>

/* tags */
#include <taglib/audioproperties.h>
#include <taglib/tbytevector.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/id3v2header.h>
#include <taglib/commentsframe.h>
#include <taglib/id3v1tag.h>
#include <taglib/attachedpictureframe.h>
#include <taglib/apetag.h>
#include <taglib/xiphcomment.h>

/* file types */
#include<taglib/asffile.h>
#include<taglib/mpegfile.h>
#include<taglib/vorbisfile.h>
#include<taglib/flacfile.h>
#include<taglib/oggflacfile.h>
#include<taglib/mpcfile.h>
#include<taglib/mp4file.h>
#include<taglib/wavpackfile.h>
#include<taglib/speexfile.h>
#include<taglib/opusfile.h>
#include<taglib/trueaudiofile.h>
#include<taglib/aifffile.h>
#include<taglib/wavfile.h>
#include<taglib/apefile.h>
#include<taglib/modfile.h>
#include<taglib/s3mfile.h>
#include<taglib/itfile.h>
#include<taglib/xmfile.h>
#include<taglib/mp4file.h>


#ifdef _WIN32
#include <Windows.h>
#pragma comment (lib, "bass.lib")
#pragma comment (lib, "tag.lib")
#pragma comment (lib, "tag_c.lib")
#else
#endif

using namespace TagLib;
using namespace std;
using namespace nlohmann;

extern "C" {
	int deviceCount = 0;
	HSTREAM stream = 0;

	const char* cvt2utf(const char* src) {
#ifdef _WIN32
		int len = MultiByteToWideChar(CP_ACP, 0, src, -1, NULL, 0);
		wchar_t* wstr = new wchar_t[len + 1];
		memset(wstr, 0, len + 1);
		MultiByteToWideChar(CP_ACP, 0, src, -1, wstr, len);
		len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		char* str = new char[len + 1];
		memset(str, 0, len + 1);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);

		if (wstr) delete[] wstr;
		return str;
#else
		return src;
#endif
	}

	const char* cvt2gbk(const char* src) {
#ifdef _WIN32
		int len = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);
		wchar_t* wszGBK = new wchar_t[len + 1];
		memset(wszGBK, 0, len * 2 + 2);
		MultiByteToWideChar(CP_UTF8, 0, src, -1, wszGBK, len);
		len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
		char* szGBK = new char[len + 1];
		memset(szGBK, 0, len + 1);
		WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);

		if (wszGBK) delete[] wszGBK;
		return szGBK;
#else
		return src;
#endif
	}

	FLYAUDIO_API void init(int device, int freq) {
		// BASS_Init(-1, 44100, 0, 0, 0);
		BASS_Init(device, freq, 0, 0, 0);
	}

	FLYAUDIO_API DEVICE_INFO** getDevices() {
		int a;
		deviceCount = 0;
		vector<DEVICE_INFO*> vec;
		BASS_DEVICEINFO info;
		BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, true);
		for (a = 1; BASS_GetDeviceInfo(a, &info); a++) {
			if (info.flags & BASS_DEVICE_ENABLED) {
				deviceCount++;
				DEVICE_INFO* dinfo = new DEVICE_INFO();
				dinfo->driver = info.driver;
				dinfo->name = info.name;
				dinfo->flags = info.flags;
				dinfo->isDefault = (info.flags & BASS_DEVICE_DEFAULT) == 0 ? 0 : 1;
				vec.push_back(dinfo);
			}
		}

		DEVICE_INFO** ret = new DEVICE_INFO * [deviceCount];
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
		}
		else {
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
		float _vol = (float)(vol < 0 ? 0 : (vol > 100 ? 100 : vol)) / (float)100.0;
		BASS_ChannelSetAttribute(stream, BASS_ATTRIB_VOL, _vol);
		return getVolume();
	}

	FLYAUDIO_API int getVolume() {
		float vol;
		BASS_ChannelGetAttribute(stream, BASS_ATTRIB_VOL, &vol);
		return (int)(100.0 * vol);
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

	/* Audio meta and properties */

	const char* dumpStr(json j) {
		string json_str = j.dump(-1, ' ', false, nlohmann::detail::error_handler_t::ignore);
		char* chr = new char[json_str.length() + 1];
		for (unsigned long long i = 0; i < json_str.length(); i++) {
			chr[i] = json_str[i];
		}
		chr[json_str.length()] = '\0';
		return chr;
	}

	FLYAUDIO_API const char* audioTags(const char* file) {
		FileRef f(file, true, AudioProperties::ReadStyle::Average);
		json j;
		PropertyMap map = f.tag()->properties();
		for (PropertyMap::ConstIterator it = map.begin(); it != map.end(); it++) {
			j[it->first.toCString(true)] = it->second.toString().toCString(true);
		}
		return dumpStr(j);
	}

	FLYAUDIO_API const char* audioProperties(const char* file) {
		FileRef f(file, true, AudioProperties::ReadStyle::Accurate);
		json j;
		PropertyMap map = f.file()->properties();
		for (PropertyMap::ConstIterator it = map.begin(); it != map.end(); it++) {
			j[it->first.toCString(true)] = it->second.toString().toCString(true);
		}
		return dumpStr(j);
	}

	/*

	  enum Type {
		//! A type not enumerated below
		Other              = 0x00,
		//! 32x32 PNG image that should be used as the file icon
		FileIcon           = 0x01,
		//! File icon of a different size or format
		OtherFileIcon      = 0x02,
		//! Front cover image of the album
		FrontCover         = 0x03,
		//! Back cover image of the album
		BackCover          = 0x04,
		//! Inside leaflet page of the album
		LeafletPage        = 0x05,
		//! Image from the album itself
		Media              = 0x06,
		//! Picture of the lead artist or soloist
		LeadArtist         = 0x07,
		//! Picture of the artist or performer
		Artist             = 0x08,
		//! Picture of the conductor
		Conductor          = 0x09,
		//! Picture of the band or orchestra
		Band               = 0x0A,
		//! Picture of the composer
		Composer           = 0x0B,
		//! Picture of the lyricist or text writer
		Lyricist           = 0x0C,
		//! Picture of the recording location or studio
		RecordingLocation  = 0x0D,
		//! Picture of the artists during recording
		DuringRecording    = 0x0E,
		//! Picture of the artists during performance
		DuringPerformance  = 0x0F,
		//! Picture from a movie or video related to the track
		MovieScreenCapture = 0x10,
		//! Picture of a large, coloured fish
		ColouredFish       = 0x11,
		//! Illustration related to the track
		Illustration       = 0x12,
		//! Logo of the band or performer
		BandLogo           = 0x13,
		//! Logo of the publisher (record company)
		PublisherLogo      = 0x14
	  };

	*/
	const char* audioCoverType(int type) {
		switch (type) {
		case 0x00: return "Other";
		case 0x01: return "FileIcon";
		case 0x02: return "OtherFileIcon";
		case 0x03: return "FrontCover";
		case 0x04: return "BackCover";
		case 0x05: return "LeafletPage";
		case 0x06: return "Media";
		case 0x07: return "LeadArtist";
		case 0x08: return "Artist";
		case 0x09: return "Conductor";
		case 0x0A: return "Band";
		case 0x0B: return "Composer";
		case 0x0C: return "Lyricist";
		case 0x0D: return "RecordingLocation";
		case 0x0E: return "DuringRecording";
		case 0x0F: return "DuringPerformance";
		case 0x10: return "MovieScreenCapture";
		case 0x11: return "ColouredFish";
		case 0x12: return "Illustration";
		case 0x13: return "BandLogo";
		case 0x14: return "PublisherLogo";
		default:
			break;
		}
		return "";
	}

	ByteVector fixArtBinary(ByteVector vec) {
		// 第一个有效字节下标
		int pos = -1;
		if (vec[0] == 'C' || vec[0] == 'c')
			if (vec[1] == 'O' || vec[1] == 'o')
				if (vec[2] == 'V' || vec[2] == 'v')
					if (vec[3] == 'E' || vec[3] == 'e')
						if (vec[4] == 'R' || vec[4] == 'r')
							pos = vec.find('\0') + 1;
		if (pos > -1) return ByteVector(vec, pos, vec.size() - pos);
		return vec;
	}

	const char* audioArtExt(ByteVector vec) {
		unsigned char* jpg = new unsigned char[2]{ 0xFF, 0xD8 };
		unsigned char* png = new unsigned char[8]{ 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
		unsigned char* bmp = new unsigned char[2]{ 0x42, 0x4d };
		bool flag = 1;
		for (int i = 0; i < 2; i++) {
			if ((int)(unsigned char)vec[i] != jpg[i]) {
				flag = 0;
				break;
			}
		}
		if (flag) return "jpg";

		flag = 1;
		for (int i = 0; i < 8; i++) {
			if ((int)(unsigned char)vec[i] != png[i]) {
				flag = 0;
				break;
			}
		}
		if (flag) return "png";

		flag = 1;
		for (int i = 0; i < 2; i++) {
			if ((int)(unsigned char)vec[i] != jpg[i]) {
				flag = 0;
				break;
			}
		}
		if (flag) return "bmp";
		return "img";
	}

	const char* audioMime(ByteVector vec) {
		const char* ext = audioArtExt(vec);
		if (strlen(ext) == 0) {
			return "image/";
		}
		else if (ext[0] == 'j') {
			return "image/jpeg";
		}
		else if (ext[0] == 'p') {
			return "image/png";
		}
		else return "image/bmp";
	}

	const char* getAudioArtsInternalId3v2(FileRef ref, ID3v2::Tag* id3v2, const char* file, const char* cacheDir, int bin) {
		json j;
		int count = 0;
		ID3v2::FrameList apics = id3v2->frameListMap()["APIC"];
		for (int i = 0; i < apics.size(); i++) {
			if (dynamic_cast<const TagLib::ID3v2::AttachedPictureFrame*>(apics[i])) {
				const ID3v2::AttachedPictureFrame* frm = dynamic_cast<const ID3v2::AttachedPictureFrame*>(apics[i]);

				json item;
				item["type"] = audioCoverType(frm->type());
				item["comment"] = frm->description().toCString(true);
				item["mime"] = frm->mimeType().toCString(true);

				if (cacheDir != NULL && strlen(cacheDir) > 0) {
					// calculate md5sum
					string cacheFileName = cacheDir;
					if (cacheDir[strlen(cacheDir) - 1] != '\\' && cacheDir[strlen(cacheDir) - 1] != '/') {
#ifdef _WIN32
						cacheFileName += "\\";
#else
						cacheFileName += "/";
#endif // _WIN32
					}
					cacheFileName += MD5::MD5(file).toStr();
					cacheFileName += "_" + string(audioCoverType(frm->type()));
					cacheFileName += "." + string(audioArtExt(frm->picture()));

					FILE* fp = fopen(cacheFileName.c_str(), "wb");
					fwrite(frm->picture().data(), sizeof(char), frm->picture().size(), fp);
					fclose(fp);
					item["file"] = cacheFileName.c_str();
				}
				else {
					if (bin == 0) {
						return frm->picture().data();
					}
					else if (bin == 1) {
						// 输出图片的Base64
						return frm->picture().toBase64().data();
					}
					else {
						item["file"] = frm->picture().toBase64().data();
					}
				}

				j["list"][count++] = item;
			}
		}
		j["count"] = count;
		return count == 0 ? "" : dumpStr(j);
	}

	const char* getAudioArtsInternalFlac(FLAC::File* flac, const char* file, const char* cacheDir, int bin) {
		json j;
		int count = 0;
		List<FLAC::Picture*> pics = flac->pictureList();
		for (int i = 0; i < pics.size(); i++) {
			json item;
			item["type"] = audioCoverType(pics[i]->type());
			item["comment"] = pics[i]->description().toCString(true);
			item["mime"] = pics[i]->mimeType().toCString(true);

			if (cacheDir != NULL && strlen(cacheDir) > 0) {
				// calculate md5sum
				string cacheFileName = cacheDir;
				if (cacheDir[strlen(cacheDir) - 1] != '\\' && cacheDir[strlen(cacheDir) - 1] != '/') {
#ifdef _WIN32
					cacheFileName += "\\";
#else
					cacheFileName += "/";
#endif // _WIN32
				}
				cacheFileName += MD5::MD5(file).toStr();
				cacheFileName += "_" + string(audioCoverType(pics[i]->type()));
				cacheFileName += "." + string(audioArtExt(pics[i]->data()));

				FILE* fp = fopen(cacheFileName.c_str(), "wb");
				fwrite(pics[i]->data().data(), sizeof(char), pics[i]->data().size(), fp);
				fclose(fp);
				item["file"] = cacheFileName.c_str();
			}
			else {
				if (bin == 0) {
					return pics[i]->data().data();
				}
				else if (bin == 1) {
					// 输出图片的Base64
					return pics[i]->data().toBase64().data();
				}
				else {
					item["file"] = pics[i]->data().toBase64().data();
				}
			}

			j["list"][count++] = item;
		}

		j["count"] = count;
		return count == 0 ? "\0" : dumpStr(j);
	}

	const char* getAudioArtsInternalXiph(Ogg::XiphComment* xiph, const char* file, const char* cacheDir, int bin) {
		json j;
		int count = 0;
		return count == 0 ? "" : dumpStr(j);
	}

	const char* getAudioArtsInternalApe(FileRef ref, APE::Tag* ape, const char* file, const char* cacheDir, int bin) {
		json j;
		int count = 0;
		for (APE::ItemListMap::ConstIterator it = ape->itemListMap().begin();
			it != ape->itemListMap().end(); ++it) {
			if ((*it).second.type() == APE::Item::Binary) {
				if ((*it).first.upper().startsWith(String("COVER ART"))) {
					ByteVector picBin = fixArtBinary((*it).second.binaryData());
					json item;

					int typeStartPos = (*it).first.find("(");
					int typeEndPos = typeStartPos > -1 ? (*it).first.find(")") : -1;
					string type = typeStartPos > -1 ? (*it).first.upper().substr(typeStartPos + 1, typeEndPos - typeStartPos - 1).toCString(true) : "";

					item["type"] = type;
					item["comment"] = "";
					item["mime"] = audioMime(picBin);

					if (cacheDir != NULL && strlen(cacheDir) > 0) {
						// calculate md5sum
						string cacheFileName = cacheDir;
						if (cacheDir[strlen(cacheDir) - 1] != '\\' && cacheDir[strlen(cacheDir) - 1] != '/') {
#ifdef _WIN32
							cacheFileName += "\\";
#else
							cacheFileName += "/";
#endif // _WIN32
						}
						cacheFileName += MD5::MD5(file).toStr();
						cacheFileName += "_" + type;
						cacheFileName += "." + string(audioArtExt(picBin));

						FILE* fp = fopen(cacheFileName.c_str(), "wb");
						fwrite(picBin.data(), sizeof(char), picBin.size(), fp);
						fclose(fp);
						item["file"] = cacheFileName.c_str();
					}
					else {
						if (bin == 0) {
							return picBin.data();
						}
						else if (bin == 1) {
							// 输出图片的Base64
							return picBin.toBase64().data();
						}
						else {
							item["file"] = picBin.toBase64().data();
						}
					}

					j["list"][count++] = item;
				}
			}
		}
		j["count"] = count;
		return count == 0 ? "\0" : dumpStr(j);
	}

	/*
	 *返回JSON格式的音频封面
	 * {"count": 1, "list": [ { "type": 3, "file": "/cache/dir/img.jpg", "comment": "", "mime": "image/png" } ]}
	 * 若cacheDir为空，bin == 0时返回第一张封面的二进制字节数组；bin == 1时输出第一张图片的Base64；bin为其它值时，将json中的file填充图片的Base64
	 */
	FLYAUDIO_API const char* audioArts(const char* filePath, const char* cacheDir, int bin) {
		FileRef ref(filePath);
		File* f = ref.file();
		char* res = NULL;
		if (dynamic_cast<APE::File*>(f)) {
			APE::File* file = dynamic_cast<APE::File*>(f);
			if (file->hasAPETag()) {
				const char* ret = getAudioArtsInternalApe(ref, file->APETag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<FLAC::File*>(f)) {
			FLAC::File* file = dynamic_cast<FLAC::File*>(f);
			const char* ret = getAudioArtsInternalFlac(file, filePath, cacheDir, bin);
			res = new char[strlen(ret) + 1];
			strcpy_s(res, strlen(ret) + 1, ret);
			res[strlen(ret)] = '\0';

			if (strlen(ret) == 0 && file->hasID3v2Tag()) {
				ret = getAudioArtsInternalId3v2(ref, file->ID3v2Tag(), filePath, cacheDir, bin);
				delete[] res;
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<IT::File*>(f)) {
		}
		else if (dynamic_cast<Mod::File*>(f)) {

		}
		else if (dynamic_cast<MPC::File*>(f)) {
			MPC::File* file = dynamic_cast<MPC::File*>(f);
			if (file->hasAPETag()) {
				const char* ret = getAudioArtsInternalApe(ref, file->APETag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<MPEG::File*>(f)) {
			MPEG::File* file = dynamic_cast<MPEG::File*>(f);
			if (file->hasID3v2Tag()) {
				const char* ret = getAudioArtsInternalId3v2(ref, file->ID3v2Tag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
			if (strlen(res) == 0 && file->hasAPETag()) {
				const char* ret = getAudioArtsInternalApe(ref, file->APETag(), filePath, cacheDir, bin);
				delete[] res;
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<Ogg::FLAC::File*>(f)) {
			Ogg::FLAC::File* file = dynamic_cast<Ogg::FLAC::File*>(f);
			if (file->hasXiphComment()) {
				const char* ret = getAudioArtsInternalXiph(file->tag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<Ogg::Speex::File*>(f)) {
			Ogg::Speex::File* file = dynamic_cast<Ogg::Speex::File*>(f);
			const char* ret = getAudioArtsInternalXiph(file->tag(), filePath, cacheDir, bin);
			res = new char[strlen(ret) + 1];
			strcpy_s(res, strlen(ret) + 1, ret);
			res[strlen(ret)] = '\0';
		}
		else if (dynamic_cast<Ogg::Opus::File*>(f)) {
			Ogg::Opus::File* file = dynamic_cast<Ogg::Opus::File*>(f);
			const char* ret = getAudioArtsInternalXiph(file->tag(), filePath, cacheDir, bin);
			res = new char[strlen(ret) + 1];
			strcpy_s(res, strlen(ret) + 1, ret);
			res[strlen(ret)] = '\0';
		}
		else if (dynamic_cast<Ogg::Vorbis::File*>(f)) {
			Ogg::Vorbis::File* file = dynamic_cast<Ogg::Vorbis::File*>(f);
			const char* ret = getAudioArtsInternalXiph(file->tag(), filePath, cacheDir, bin);
			res = new char[strlen(ret) + 1];
			strcpy_s(res, strlen(ret) + 1, ret);
			res[strlen(ret)] = '\0';
		}
		else if (dynamic_cast<RIFF::AIFF::File*>(f)) {
			RIFF::AIFF::File* file = dynamic_cast<RIFF::AIFF::File*>(f);
			if (file->hasID3v2Tag()) {
				const char* ret = getAudioArtsInternalId3v2(ref, file->tag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<RIFF::WAV::File*>(f)) {
			RIFF::WAV::File* file = dynamic_cast<RIFF::WAV::File*>(f);
			if (file->hasID3v2Tag()) {
				const char* ret = getAudioArtsInternalId3v2(ref, file->tag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<S3M::File*>(f)) {
		}
		else if (dynamic_cast<TrueAudio::File*>(f)) {
			TrueAudio::File* file = dynamic_cast<TrueAudio::File*>(f);
			if (file->hasID3v2Tag()) {
				const char* ret = getAudioArtsInternalId3v2(ref, file->ID3v2Tag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<WavPack::File*>(f)) {
			WavPack::File* file = dynamic_cast<WavPack::File*>(f);
			if (file->hasAPETag()) {
				const char* ret = getAudioArtsInternalApe(ref, file->APETag(), filePath, cacheDir, bin);
				res = new char[strlen(ret) + 1];
				strcpy_s(res, strlen(ret) + 1, ret);
				res[strlen(ret)] = '\0';
			}
		}
		else if (dynamic_cast<XM::File*>(f)) {
		}
		else if (dynamic_cast<MP4::File*>(f)) {
		}
		else if (dynamic_cast<ASF::File*>(f)) {
		}

		if (res == NULL || strlen(res) == 0) return "{\"count\": 0}";
		return res;
	}
}