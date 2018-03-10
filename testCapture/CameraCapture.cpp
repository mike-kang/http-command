#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include "CameraCapture.h"

#define HTTP_HEAD_FMT "GET /cgi-bin/httpapi/storage.cgi?svc=stillImage&action=exec HTTP/1.1\r\n" \
		"Authorization: Basic Og==\r\n" \
		"Host: %s\r\n" \
		"Connection: Keep-Alive\r\n" \
		"Cache-Control: no-cache\r\n\r\n" 

#define MIN(a,b) ((a)<(b)? (a) : (b)) 

CameraCapture::CameraCapture(const char* ip, const char* name)
{
	strcpy(m_ip, ip);
	strcpy(m_name, name);

	ZeroMemory(&m_serveraddr, sizeof(m_serveraddr));
	m_serveraddr.sin_family = AF_INET;
	m_serveraddr.sin_addr.s_addr = inet_addr(m_ip);
	m_serveraddr.sin_port = htons(80);

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) throw 1;
}

bool CameraCapture::connect()
{
	int retval;

	retval = ::connect(m_sock, (SOCKADDR *)&m_serveraddr, sizeof(m_serveraddr));
	if (retval == SOCKET_ERROR) {
		closesocket(m_sock);
		return false;
	}
	return true;
}

bool CameraCapture::capture()
{
	int retval;
	// 데이터 보내기
	char buf[4096];

	sprintf(buf, HTTP_HEAD_FMT, m_ip);
	retval = send(m_sock, buf, strlen(buf), 0);
	if (retval == SOCKET_ERROR) {
		printf("get error %d\n", WSAGetLastError());
		return false;
	}

	//to get jpeg size
	int left = 1024;
	int received;
	char *p = buf;
	while (left > 0) {
		received = recv(m_sock, p, left, 0);
		if (received == SOCKET_ERROR) {
			return false;;
		}
		else if (received == 0) {
			return false;
		}
		left -= received;
		p += received;
	}
	buf[1024] = '\0';
	p = strstr(buf, "Content-Length:");
	if (!p)
		return false;
	m_img.len = atoi(p + strlen("Content-Length:"));
	char* image = new char[m_img.len];
	p = strstr(p, "\r\n\r\n");
	int pre_read = 1024 - ((p - buf) + 4);
	memcpy(image, p + 4, pre_read);

	left = m_img.len - pre_read;
	p = image + pre_read;
	while (left > 0) {
		received = recv(m_sock, p, MIN(left, 4096), 0);
		if (received == SOCKET_ERROR) {
			return false;
		}
		else if (received == 0) {
			return false;
		}
		left -= received;
		p += received;
	}
	m_img.data = image;

	return true;
}

image_t* CameraCapture::getImage()
{
	return &m_img;
}

void CameraCapture::releaseImageData()
{
	delete [] m_img.data;
	m_img.data = NULL;
}

CameraCapture::~CameraCapture()
{
	printf("~CameraCapture\n");
	closesocket(m_sock);
}
