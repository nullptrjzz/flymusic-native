// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 FLYAUDIO_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// FLYAUDIO_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。

#ifdef _WIN32

#ifdef FLYAUDIO_EXPORTS
#define FLYAUDIO_API __declspec(dllexport)
#else
#define FLYAUDIO_API __declspec(dllimport)
#endif

#else

#define FLYAUDIO_API

#endif // _WIN32


struct DEVICE_INFO {
#if defined(_WIN32_WCE) || (WINAPI_FAMILY && WINAPI_FAMILY!=WINAPI_FAMILY_DESKTOP_APP)
	const wchar_t* name;	// description
	const wchar_t* driver;	// driver
#else
	const char* name;	// description
	const char* driver;	// driver
#endif
	unsigned long long flags;
	bool isDefault;
};

/* 记录音频文件信息，来自ID3(v2)标签 */
struct AUDIO_META {
	// basic info
	int length; // s
	int bitRate; // kb/s
	int channels;
	int sampleRate; // Hz

	// id3 tags
	const char* title;
	const char* artist;
	const char* album;
	const char* comment;
	const char* genre;
	int year;
	int track;

	// other id3 tags
	const char* albumartist;
	const char* subtitle;
	int discnumber;
	int date;
	int originaldate;
};

extern "C" {
	/* Player control */
	FLYAUDIO_API void init(int, int);
	FLYAUDIO_API DEVICE_INFO** getDevices();
	FLYAUDIO_API int getDeviceCount();
	FLYAUDIO_API bool setDevice(int);
	FLYAUDIO_API int loadFile(const char*);

	FLYAUDIO_API bool play();
	FLYAUDIO_API bool pause();
	FLYAUDIO_API bool stop();

	FLYAUDIO_API double getDuration();

	FLYAUDIO_API double getPosition();
	FLYAUDIO_API void setPosition(double);

	FLYAUDIO_API unsigned long long getDurationB();
	FLYAUDIO_API unsigned long long getPositionB();
	FLYAUDIO_API void setPositionB(unsigned long long);

	FLYAUDIO_API int setVolume(int);
	FLYAUDIO_API int getVolume();

	FLYAUDIO_API float getCpu();

	FLYAUDIO_API void freeStream();
	FLYAUDIO_API void close();

	/* Audio meta */
	FLYAUDIO_API AUDIO_META* audioMeta(const char* file);
}