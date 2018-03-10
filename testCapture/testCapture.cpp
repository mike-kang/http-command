// testCapture.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//
#include "stdafx.h"
#include <time.h>
#include "CameraCapture.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32")

using namespace std;

char* make_name(const char*ip)
{
	static char buf[100];
	strncpy(buf, ip, 100);
	char* p = buf;
	while (*p) {
		if (*p == '.')
			*p = '_';
		p++;
	}
	strcat(buf, ".jpg");
	return buf;
}

static void do_capture(CameraCapture* cc)
{
	cc->capture();

	FILE* fp = fopen(cc->getName(), "wb");
	image_t* img = cc->getImage();
	fwrite(img->data, img->len, 1, fp);
	fclose(fp);
}

int main(int argc, char *argv[])
{
	int retval;
	vector<string> vec_ip;
	vector<CameraCapture*> vec_cc;
	vector<thread*> vec_thread;

	if (argc < 1) {
		printf("%s ip or %s list.txt\n");
		return 0;
	}
	
	if (!strcmp(argv[1], "list.txt")) {
		ifstream infile("list.txt");
		string line;
		
		while(std::getline(infile, line))
		{
			vec_ip.push_back(line);
		}
	}
	else {
		vec_ip.push_back(argv[1]);
	}
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	for (auto ip : vec_ip) {
		char * name = make_name(ip.c_str());
		CameraCapture* cc = new CameraCapture(ip.c_str(), name);
		vec_cc.push_back(cc);
	}

	//connect
	for (auto cc : vec_cc) {
		cc->connect();
	}

	clock_t current_tick = clock();
	//capture
	for (auto cc : vec_cc) {
		vec_thread.push_back(new thread(do_capture, cc));
	}

	//join
	for (int i = 0; i < vec_thread.size(); i++) {
		vec_thread[i]->join();
	}
	printf("[time] %f\n", (clock() - current_tick) / (double)CLOCKS_PER_SEC);

	//release
	for (auto cc : vec_cc) {
		cc->releaseImageData();
	}
	

	// ���� ����
	WSACleanup();
	return 0;
}