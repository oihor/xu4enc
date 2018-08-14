
#include <poll.h>

#include "M2M.h"

#define V4L2_CID_MPEG_MFC_BASE      (V4L2_CTRL_CLASS_MPEG | 0x2000)
#define V4L2_CID_MPEG_MFC51_VIDEO_H264_RC_FRAME_RATE    \
                    (V4L2_CID_MPEG_MFC_BASE + 21)

ssize_t read_file_to_buf(int fd, void *buf, size_t nbytes) {
    // get buffer
    ssize_t totalRead = 0;
    while (totalRead < nbytes)
    {
        ssize_t readCount = read(fd, (char*)(buf) + totalRead, nbytes - totalRead);
        if (readCount <= 0)
        {
            //throw Exception("read failed.");

            // End of stream?
            fprintf(stderr, "read_file_to_buf read failed. (%d)\n", readCount);

            // TODO: Signal codec and flush
            break;
        }
        else
        {
            totalRead += readCount;
        }
    }

    if (totalRead < nbytes)
    {
        fprintf(stderr, "read_file_to_buf read underflow. (%d of %d)\n", totalRead, nbytes);
    }

    return totalRead;
}



	void M2M::SetProfile()
	{
		//V4L2_CID_MPEG_VIDEO_H264_PROFILE = V4L2_MPEG_VIDEO_H264_PROFILE_HIGH
		//V4L2_CID_MPEG_VIDEO_H264_LEVEL = V4L2_MPEG_VIDEO_H264_LEVEL_4_0

#if 1
		v4l2_ext_control ctrl[2] = { 0 };
		ctrl[0].id = V4L2_CID_MPEG_VIDEO_H264_PROFILE;
		ctrl[0].value = V4L2_MPEG_VIDEO_H264_PROFILE_MAIN;

		ctrl[1].id = V4L2_CID_MPEG_VIDEO_H264_LEVEL;
		ctrl[1].value = V4L2_MPEG_VIDEO_H264_LEVEL_4_0;

		v4l2_ext_controls ctrls = { 0 };
		ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
		ctrls.count = 2;
		ctrls.controls = ctrl;

		int io = ioctl(mfc_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
		if (io != 0)
		{
			throw Exception("VIDIOC_S_EXT_CTRLS failed.");
		}
#endif

	}

	void M2M::SetBitrate(int value)
	{
		v4l2_ext_control ctrl[2] = { 0 };
		ctrl[0].id = V4L2_CID_MPEG_VIDEO_BITRATE;
		ctrl[0].value = value;

		ctrl[1].id = V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE;
		ctrl[1].value = 1;

		v4l2_ext_controls ctrls = { 0 };
		ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
		ctrls.count = 2;
		ctrls.controls = ctrl;

		int io = ioctl(mfc_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
		if (io != 0)
		{
			throw Exception("VIDIOC_S_EXT_CTRLS failed.");
		}
	}

	void M2M::SetFPS(int value) {
        v4l2_ext_control ctrl[2] = { 0 };

        ctrl[0].id = V4L2_CID_MPEG_VIDEO_FRAME_RC_ENABLE;
        ctrl[0].value = 1;

        ctrl[1].id = V4L2_CID_MPEG_MFC51_VIDEO_H264_RC_FRAME_RATE;
        ctrl[1].value = fps;

        v4l2_ext_controls ctrls = { 0 };
        ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
        ctrls.count = 2;
        ctrls.controls = ctrl;

        int io = ioctl(mfc_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
        if (io != 0)
        {
            throw Exception("VIDIOC_S_EXT_CTRLS failed.");
        }
	}

	void M2M::SetGroupOfPictures(int value)
	{
		v4l2_ext_control ctrl = { 0 };
		ctrl.id = V4L2_CID_MPEG_VIDEO_GOP_SIZE;
		ctrl.value = value;

		v4l2_ext_controls ctrls = { 0 };
		ctrls.ctrl_class = V4L2_CTRL_CLASS_MPEG;
		ctrls.count = 1;
		ctrls.controls = &ctrl;

		int io = ioctl(mfc_fd, VIDIOC_S_EXT_CTRLS, &ctrls);
		if (io != 0)
		{
			throw Exception("VIDIOC_S_EXT_CTRLS failed.");
		}
	}

	void M2M::ApplyInputSettings()
	{
		// Apply capture settings
		v4l2_format format = { 0 };
		format.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		format.fmt.pix_mp.width = width;
		format.fmt.pix_mp.height = height;
		format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_NV12M;
		format.fmt.pix_mp.num_planes = 2;
		//format.fmt.pix_mp.plane_fmt[0].sizeimage = width * height;
		//format.fmt.pix_mp.plane_fmt[1].sizeimage = width * height / 2;

		fprintf(stderr, "v4l2_format in about to set: width=%d, height=%d, pixelformat=0x%x\n",
			format.fmt.pix.width, format.fmt.pix.height, format.fmt.pix.pixelformat);

		int io = ioctl(mfc_fd, VIDIOC_S_FMT, &format);
		if (io < 0)
		{
			throw Exception("VIDIOC_S_FMT failed.");
		}

		fprintf(stderr, "v4l2_format in has been set: width=%d, height=%d, pixelformat=0x%x\n",
			format.fmt.pix.width, format.fmt.pix.height, format.fmt.pix.pixelformat);


//		v4l2_streamparm streamParm = { 0 };
//		streamParm.type = format.type;
//		streamParm.parm.capture.timeperframe.numerator = 1;
//		streamParm.parm.capture.timeperframe.denominator = fps;
//
//		io = ioctl(mfc_fd, VIDIOC_S_PARM, &streamParm);
//		if (io < 0)
//		{
//			throw Exception("VIDIOC_S_PARM failed.");
//		}
//
//		fprintf(stderr, "capture.timeperframe: numerator=%d, denominator=%d\n",
//			streamParm.parm.capture.timeperframe.numerator,
//			streamParm.parm.capture.timeperframe.denominator);


		SetProfile();

		SetFPS(fps);

		SetBitrate(bitrate);
		
		SetGroupOfPictures(gop);
	}

	void M2M::CreateInputBuffers()
	{
		// Request buffers
		v4l2_requestbuffers requestBuffers = { 0 };
		requestBuffers.count = OUTPUT_BUFFER_COUNT;
		requestBuffers.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		requestBuffers.memory = V4L2_MEMORY_MMAP;

		int io = ioctl(mfc_fd, VIDIOC_REQBUFS, &requestBuffers);
		if (io < 0)
		{
			throw Exception("OUTPUT VIDIOC_REQBUFS failed.");
		}

		if (requestBuffers.count > OUTPUT_BUFFER_COUNT)
		{
			throw Exception("too many buffers.");
		}
		else
		{
			fprintf(stderr, "CreateInputBuffers: requestBuffers.count=%d\n", requestBuffers.count);
		}


		// Map buffers
		//BufferMapping bufferMappings[requestBuffers.count] = { 0 };
		for (int i = 0; i < requestBuffers.count; ++i)
		{
			v4l2_plane planes[VIDEO_MAX_PLANES];

			v4l2_buffer buffer = { 0 };
			buffer.type = requestBuffers.type;
			buffer.memory = V4L2_MEMORY_MMAP;
			buffer.index = i;
			buffer.m.planes = planes;
			buffer.length = 2;

			io = ioctl(mfc_fd, VIDIOC_QUERYBUF, &buffer);
			if (io < 0)
			{
				throw Exception("VIDIOC_QUERYBUF failed.");
			}

			BufferMapping mapping = { 0 };

			mapping.Length0 = buffer.m.planes[0].length;
			mapping.Start0 = mmap(NULL, mapping.Length0,
				PROT_READ | PROT_WRITE, /* recommended */
				MAP_SHARED,             /* recommended */
				mfc_fd,
				buffer.m.planes[0].m.mem_offset);
			if (mapping.Start0 == MAP_FAILED)
			{
				throw Exception("mmap 0 failed.");
			}

			mapping.Length1 = buffer.m.planes[1].length;
			mapping.Start1 = mmap(NULL, mapping.Length1,
				PROT_READ | PROT_WRITE, /* recommended */
				MAP_SHARED,             /* recommended */
				mfc_fd,
				buffer.m.planes[1].m.mem_offset);
			if (mapping.Start1 == MAP_FAILED)
			{
				throw Exception("mmap 1 failed.");
			}

			fprintf(stderr, "Adding buffer %d Length0 %zx Start0 %p Length1 %zx Start1 %p \n", i, mapping.Length0, mapping.Start0, mapping.Length1, mapping.Start1);

			inputBuffers.push_back(mapping);
			freeInputBuffers.push(i);

#if 0 // Dont queue input buffers, only output buffers

			// Queue buffers
			int ret = ioctl(mfc_fd, VIDIOC_QBUF, &buffer);
			if (ret != 0)
			{
				throw Exception("VIDIOC_QBUF failed.");
			}
#endif

		}
	}

	void M2M::ApplyOutputSettings()
	{
		// Apply capture settings
		v4l2_format format = { 0 };
		format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		format.fmt.pix_mp.width = width;
		format.fmt.pix_mp.height = height;
		format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_H264;
		format.fmt.pix_mp.plane_fmt[0].sizeimage = width * height * 1;

		fprintf(stderr, "v4l2_format out about to set: width=%d, height=%d, pixelformat=0x%x\n",
			format.fmt.pix.width, format.fmt.pix.height, format.fmt.pix.pixelformat);

		int io = ioctl(mfc_fd, VIDIOC_S_FMT, &format);
		if (io < 0)
		{
			throw Exception("VIDIOC_S_FMT failed.");
		}

		fprintf(stderr, "v4l2_format out has been set: width=%d, height=%d, pixelformat=0x%x\n",
			format.fmt.pix.width, format.fmt.pix.height, format.fmt.pix.pixelformat);

	}

	void M2M::CreateOutputBuffers()
	{
		// Request buffers
		v4l2_requestbuffers requestBuffers = { 0 };
		requestBuffers.count = CAPTURE_BUFFER_COUNT;
		requestBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		requestBuffers.memory = V4L2_MEMORY_MMAP;

		int io = ioctl(mfc_fd, VIDIOC_REQBUFS, &requestBuffers);
		if (io < 0)
		{
			throw Exception("CAPTURE VIDIOC_REQBUFS failed.");
		}

		if (requestBuffers.count > CAPTURE_BUFFER_COUNT)
		{
			throw Exception("too many buffers.");
		}
		else
		{
			fprintf(stderr, "CreateOutputBuffers: requestBuffers.count=%d\n", requestBuffers.count);
		}


		// Map buffers
		//BufferMapping bufferMappings[requestBuffers.count] = { 0 };
		for (int i = 0; i < requestBuffers.count; ++i)
		{
			v4l2_plane planes[VIDEO_MAX_PLANES];

			v4l2_buffer buffer = { 0 };
			buffer.type = requestBuffers.type;
			buffer.memory = V4L2_MEMORY_MMAP;
			buffer.index = i;
			buffer.m.planes = planes;
			buffer.length = 1;

			io = ioctl(mfc_fd, VIDIOC_QUERYBUF, &buffer);
			if (io < 0)
			{
				throw Exception("VIDIOC_QUERYBUF failed.");
			}

			BufferMapping mapping = { 0 };

			mapping.Length0 = buffer.m.planes[0].length;
			mapping.Start0 = mmap(NULL, mapping.Length0,
				PROT_READ | PROT_WRITE, /* recommended */
				MAP_SHARED,             /* recommended */
				mfc_fd,
				buffer.m.planes[0].m.mem_offset);
			if (mapping.Start0 == MAP_FAILED)
			{
				throw Exception("mmap 0 failed.");
			}

			//mapping.Length1 = buffer.m.planes[1].length;
			//mapping.Start1 = mmap(NULL, mapping.Length1,
			//	PROT_READ | PROT_WRITE, /* recommended */
			//	MAP_SHARED,             /* recommended */
			//	mfc_fd,
			//	buffer.m.planes[1].m.mem_offset);
			//if (mapping.Start1 == MAP_FAILED)
			//{
			//	throw Exception("mmap 1 failed.");
			//}

			outputBuffers.push_back(mapping);

			// Queue buffers
			int ret = ioctl(mfc_fd, VIDIOC_QBUF, &buffer);
			if (ret != 0)
			{
				throw Exception("VIDIOC_QBUF failed.");
			}
		}
	}

	void M2M::EnumFormats(uint32_t v4l2BufType)
	{
		int io;

		v4l2_fmtdesc formatDesc = { 0 };
		formatDesc.type = v4l2BufType; //V4L2_BUF_TYPE_VIDEO_CAPTURE;

		fprintf(stderr, "Supported formats:\n");
		while (true)
		{
			io = ioctl(mfc_fd, VIDIOC_ENUM_FMT, &formatDesc);
			if (io < 0)
			{
				//fprintf(stderr, "VIDIOC_ENUM_FMT failed.\n");
				break;
			}

			fprintf(stderr, "\tdescription = %s, pixelformat=0x%x\n", formatDesc.description, formatDesc.pixelformat);


			v4l2_frmsizeenum formatSize = { 0 };
			formatSize.pixel_format = formatDesc.pixelformat;

			while (true)
			{
				io = ioctl(mfc_fd, VIDIOC_ENUM_FRAMESIZES, &formatSize);
				if (io < 0)
				{
					//fprintf(stderr, "VIDIOC_ENUM_FRAMESIZES failed.\n");
					break;
				}

				fprintf(stderr, "\t\twidth = %d, height = %d\n", formatSize.discrete.width, formatSize.discrete.height);


				v4l2_frmivalenum frameInterval = { 0 };
				frameInterval.pixel_format = formatSize.pixel_format;
				frameInterval.width = formatSize.discrete.width;
				frameInterval.height = formatSize.discrete.height;

				while (true)
				{
					io = ioctl(mfc_fd, VIDIOC_ENUM_FRAMEINTERVALS, &frameInterval);
					if (io < 0)
					{
						//fprintf(stderr, "VIDIOC_ENUM_FRAMEINTERVALS failed.\n");
						break;
					}

					fprintf(stderr, "\t\t\tnumerator = %d, denominator = %d\n", frameInterval.discrete.numerator, frameInterval.discrete.denominator);
					++frameInterval.index;
				}


				++formatSize.index;
			}

			++formatDesc.index;
		}
	}

	M2M::M2M(int width, int height, int fps, int bitrate, int gop)
		: width(width), height(height), fps(fps), bitrate(bitrate), gop(gop)
	{
		// O_NONBLOCK prevents deque operations from blocking if no buffers are ready
		mfc_fd = open(decoderName, O_RDWR | O_NONBLOCK, 0);
		if (mfc_fd < 0)
		{
			throw Exception("Failed to open MFC");
		}


		// Check device capabilities
		v4l2_capability cap = { 0 };

		int ret = ioctl(mfc_fd, VIDIOC_QUERYCAP, &cap);
		if (ret != 0)
		{
			throw Exception("VIDIOC_QUERYCAP failed.");
		}

		if (/*(cap.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE) == 0 ||*/
			(cap.capabilities & V4L2_CAP_STREAMING) == 0)
		{
			fprintf(stderr, "V4L2_CAP_VIDEO_M2M_MPLANE=%d\n", (cap.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE) != 0);
			fprintf(stderr, "V4L2_CAP_STREAMING=%d\n", (cap.capabilities & V4L2_CAP_STREAMING) != 0);

			throw Exception("Insufficient capabilities of MFC device.");
		}


		fprintf(stderr, "V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE formats:\n");
		EnumFormats(V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
		
		fprintf(stderr, "-------------------------------------------\n");

		fprintf(stderr, "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE formats:\n");
		EnumFormats(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);


#if 0	// only V4L2_CID_MIN_BUFFERS_FOR_OUTPUT is supported for MFC

		// Get the number of buffers required
		v4l2_control ctrl = { 0 };
		ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_OUTPUT;

		ret = ioctl(mfc_fd, VIDIOC_G_CTRL, &ctrl);
		if (ret != 0)
		{
			fprintf(stderr, "V4L2_CID_MIN_BUFFERS_FOR_OUTPUT VIDIOC_G_CTRL failed.\n");
		}
		else
		{
			fprintf(stderr, "V4L2_CID_MIN_BUFFERS_FOR_OUTPUT=%d\n", ctrl.value);
		}

		ctrl = { 0 };
		ctrl.id = V4L2_CID_MIN_BUFFERS_FOR_CAPTURE;

		ret = ioctl(mfc_fd, VIDIOC_G_CTRL, &ctrl);
		if (ret != 0)
		{
			fprintf(stderr, "V4L2_CID_MIN_BUFFERS_FOR_CAPTURE VIDIOC_G_CTRL failed.\n");
		}
		else
		{
			fprintf(stderr, "V4L2_CID_MIN_BUFFERS_FOR_CAPTURE=%d\n", ctrl.value);
		}

#endif

		ApplyInputSettings();
		CreateInputBuffers();

		ApplyOutputSettings();
		CreateOutputBuffers();

		// Start output stream
		int val = (int)V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

		ret = ioctl(mfc_fd, VIDIOC_STREAMON, &val);
		if (ret != 0)
		{
			throw Exception("VIDIOC_STREAMON failed.");
		}
	}

	M2M::~M2M()
	{
		// Stop output stream
		int val = (int)V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;

		int ret = ioctl(mfc_fd, VIDIOC_STREAMOFF, &val);
		if (ret != 0)
		{
			throw Exception("VIDIOC_STREAMOFF failed.");
		}

		// Stop input stream
		val = (int)V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;

		ret = ioctl(mfc_fd, VIDIOC_STREAMOFF, &val);
		if (ret != 0)
		{
			throw Exception("VIDIOC_STREAMOFF failed.");
		}


		close(mfc_fd);
	}


	bool M2M::EncodeNV12(const unsigned char* y, const unsigned char* uv)
	{
		bool result;

		// Reclaim any free input buffers
		v4l2_plane planes[VIDEO_MAX_PLANES];

		v4l2_buffer dqbuf = { 0 };
		dqbuf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
		dqbuf.memory = V4L2_MEMORY_MMAP;
		dqbuf.m.planes = planes;
		dqbuf.length = 2;

		int ret = 0;
//		if(freeInputBuffers.empty()) {
//	        while(ret == 0) {
	            ret = ioctl(mfc_fd, VIDIOC_DQBUF, &dqbuf);
	            if (ret != 0)
	            {
	                //fprintf(stderr, "Waiting on V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE buffer.\n");
	            }
	            else
	            {
	                freeInputBuffers.push(dqbuf.index);
	            }
//	        }
//		}

		// Encode
		if (freeInputBuffers.empty())
		{
			waitingOutputMPlane = true;
			//fprintf(stderr, "no free buffers.\n");
			result = false;
		}
		else
		{
			waitingOutputMPlane = false;
			result = true;
			
			int index = freeInputBuffers.front();
			freeInputBuffers.pop();

			v4l2_plane planes[VIDEO_MAX_PLANES];

			v4l2_buffer buffer = { 0 };
			buffer.type = dqbuf.type;
			buffer.memory = V4L2_MEMORY_MMAP;
			buffer.index = index;
			buffer.m.planes = planes;
			buffer.length = 2;

			int io = ioctl(mfc_fd, VIDIOC_QUERYBUF, &buffer);
			if (io < 0)
			{
				throw Exception("VIDIOC_QUERYBUF failed.");
			}

			// TODO validate buffer size

			// Copy data
			BufferMapping mapping = inputBuffers[buffer.index];
			
			memcpy(mapping.Start0, y, width * height);
			//read_file_to_buf(STDIN_FILENO, mapping.Start0, width * height);
			//memset(mapping.Start0, 0x00, width * height);
			buffer.m.planes[0].bytesused = width * height;

			memcpy(mapping.Start1, uv, width * height / 2);
			//read_file_to_buf(STDIN_FILENO, mapping.Start1, width * height / 2);
			//memset(mapping.Start1, 0x00, width * height / 2);
			buffer.m.planes[1].bytesused = width * height / 2;
			

			// Re-queue buffer
			ret = ioctl(mfc_fd, VIDIOC_QBUF, &buffer);
			if (ret != 0)
			{
				throw Exception("VIDIOC_QBUF failed.");
			}


			if (!streamActive)
			{
				// Start input stream
				int val = (int)V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;

				ret = ioctl(mfc_fd, VIDIOC_STREAMON, &val);
				if (ret != 0)
				{
					throw Exception("VIDIOC_STREAMON failed.");
				}
				streamActive = true;
			}
		}

		return result;
	}


	int M2M::GetEncodedData(void* dataOut)
	{
		int result;


		// deque
		v4l2_plane planes[VIDEO_MAX_PLANES];

		v4l2_buffer buffer = { 0 };
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.m.planes = planes;
		buffer.length = 1;

		int ret = ioctl(mfc_fd, VIDIOC_DQBUF, &buffer);
		if (ret != 0)
		{
			//fprintf(stderr, "Waiting on V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE buffer.\n");
			waitingCaptureMPlane = true;
			result = 0;
		}
		else
		{
			waitingCaptureMPlane = false;
			result = buffer.m.planes[0].bytesused;

			// Copy data
			BufferMapping mapping = outputBuffers[buffer.index];

			memcpy(dataOut, mapping.Start0, buffer.m.planes[0].bytesused);


			// Re-queue buffer
			ret = ioctl(mfc_fd, VIDIOC_QBUF, &buffer);
			if (ret != 0)
			{
				throw Exception("VIDIOC_QBUF failed.");
			}
		}

		return result;
	}

	void M2M::WaitOnPoll() {
	    struct pollfd poll_events;
	    int poll_state;

	    poll_events.fd = mfc_fd;
	    poll_events.events = POLLIN | POLLOUT | POLLERR;
	    poll_events.revents = 0;

	    do {
	        poll_state = poll((struct pollfd*)&poll_events, 1, VIDEO_ENCODER_POLL_TIMEOUT);
	        if (poll_state > 0) {
	            if (poll_events.revents & (POLLIN | POLLOUT)) {
	                static int revents_num = 0;
	                revents_num++;
	                if(revents_num % 100 == 0) {
	                    fprintf(stderr, "%s: Poll revents, %d \n", __func__, revents_num);
	                }
	                break;
	            } else {
	                fprintf(stderr, "%s: Poll return error \n", __func__);
	                break;
	            }
	        } else if (poll_state < 0) {
	            fprintf(stderr, "%s: Poll state error \n", __func__);
	            break;
	        } else { // poll_state == 0
	            static int timeout_num = 0;
	            timeout_num++;
	            fprintf(stderr, "%s: Poll state timeout, %d \n", __func__, timeout_num);
	        }
	    } while (poll_state == 0);
	}

