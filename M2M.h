#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <getopt.h>

#include <exception>
#include <vector>
#include <queue>
#include <cstring>

#include <linux/videodev2.h> // V4L
#include <sys/mman.h>	// mmap
#include <stdint.h>


ssize_t read_file_to_buf(int fd, void *buf, size_t nbytes);

class Exception : public std::exception
{
public:
	Exception(const char* message)
		: std::exception()
	{
		fprintf(stderr, "%s\n", message);
	}

};


struct BufferMapping
{
	void* Start0;
	size_t Length0;
	void* Start1;
	size_t Length1;
};


class M2M
{
	static const int OUTPUT_BUFFER_COUNT = 2;
	static const int CAPTURE_BUFFER_COUNT = 2;
	static const int VIDEO_ENCODER_POLL_TIMEOUT = 25; // ms

	//[    2.236569] s5p-mfc 11000000.codec:: decoder registered as /dev/video10
	//[    2.305343] s5p-mfc 11000000.codec:: encoder registered as /dev/video11
	const char* decoderName = "/dev/video7";
	
	int mfc_fd;
	int width;
	int height;
	int fps;
	int bitrate;
	int gop;

	std::vector<BufferMapping> inputBuffers;
	std::vector<BufferMapping> outputBuffers;
	bool streamActive = false;
	std::queue<int> freeInputBuffers;

	bool waitingOutputMPlane = false;
	bool waitingCaptureMPlane = false;

	void SetProfile();

	void SetBitrate(int value);

	void SetGroupOfPictures(int value);

	void ApplyInputSettings();

	void CreateInputBuffers();

	void ApplyOutputSettings();

	void CreateOutputBuffers();

	void EnumFormats(uint32_t v4l2BufType);


public:

	M2M(int width, int height, int fps, int bitrate, int gop);

	~M2M();

	bool EncodeNV12(const unsigned char* y, const unsigned char* uv);

	int GetEncodedData(void* dataOut);

	bool isWaitingOutputMPlane() const { return waitingOutputMPlane; }
	bool isWaitingCaptureMPlane() const { return waitingCaptureMPlane; }

	void WaitOnPoll();

};
