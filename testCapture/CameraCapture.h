#ifndef __CAMERA_CAPTURE_H
#define __CAMERA_CAPTURE_H
#include <winsock2.h>

struct image_t {
	char* data;
	int len;
};

class CameraCapture
{
public:
	CameraCapture(const char* ip, const char* name);
	~CameraCapture();

	bool capture();
	image_t* getImage();
	char* getName() { return m_name; }

private:
	bool _connect();

	SOCKET m_sock;
	SOCKADDR_IN m_serveraddr;
	char m_ip[16];
	image_t m_img;
	char m_name[25];
};

#endif
