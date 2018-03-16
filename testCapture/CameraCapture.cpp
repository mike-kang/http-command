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
	m_img.len = 0;
	m_img.data = NULL;
	ZeroMemory(&m_serveraddr, sizeof(m_serveraddr));
	m_serveraddr.sin_family = AF_INET;
	m_serveraddr.sin_addr.s_addr = inet_addr(m_ip);
	m_serveraddr.sin_port = htons(80);
	
}

bool CameraCapture::_connect()
{
	int retval;
	
	retval = ::connect(m_sock, (SOCKADDR *)&m_serveraddr, sizeof(m_serveraddr));
	if (retval == SOCKET_ERROR) {
		printf("connect: get error %d\n", WSAGetLastError());
		closesocket(m_sock);
		return false;
	}
	return true;
}

bool CameraCapture::capture()
{
	int retval;
	char buf[4096];

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == INVALID_SOCKET) throw 1;

	_connect();

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
			printf("get error2 %d\n", WSAGetLastError());
			return false;;
		}
		else if (received == 0) {
			printf("received2 == 0\n");
			return false;
		}
		left -= received;
		p += received;
	}
	buf[1024] = '\0';
	p = strstr(buf, "Content-Length:");
	if (!p)
		return false;
	int len = atoi(p + strlen("Content-Length:"));
	if (m_img.len < len) {
		if (!m_img.data)
			m_img.data = (char*)malloc(len);
		else
			m_img.data = (char*)realloc(m_img.data, len);
		m_img.len = len;
	}
	else if (m_img.len > len)
		m_img.len = len;

	char* image = m_img.data;
	p = strstr(p, "\r\n\r\n");
	int pre_read = 1024 - ((p - buf) + 4);
	memcpy(image, p + 4, pre_read);

	left = m_img.len - pre_read;
	p = image + pre_read;
	while (left > 0) {
		received = recv(m_sock, p, MIN(left, 4096), 0);
		if (received == SOCKET_ERROR) {
			printf("get error3 %d\n", WSAGetLastError());
			return false;
		}
		else if (received == 0) {
			printf("received == 0\n");
			return false;
		}
		left -= received;
		p += received;
	}

	closesocket(m_sock);
	return true;
}

image_t* CameraCapture::getImage()
{
	return &m_img;
}

CameraCapture::~CameraCapture()
{
	printf("~CameraCapture\n");
	free(m_img.data);
}
