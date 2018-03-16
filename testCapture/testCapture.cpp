// testCapture.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
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
	if (!cc->capture()) {
		printf("capture error!\n");
		return;
	}
	//printf("do_capture!\n");
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


	if (argc < 1) {
		printf("%s ip or %s list.txt\n");
		return 0;
	}

	if (!strcmp(argv[1], "list.txt")) {
		ifstream infile("list.txt");
		string line;

		while (std::getline(infile, line))
		{
			vec_ip.push_back(line);
		}
	}
	else {
		vec_ip.push_back(argv[1]);
	}
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	for (auto ip : vec_ip) {
		char * name = make_name(ip.c_str());
		CameraCapture* cc = new CameraCapture(ip.c_str(), name);
		vec_cc.push_back(cc);
	}
		
	for (int j = 0; j < 3; j++) {
		vector<thread*> vec_thread;
		clock_t current_tick = clock();

		//capture
		for (auto cc : vec_cc) {
			vec_thread.push_back(new thread(do_capture, cc));
		}

		//join
		for (int i = 0; i < vec_thread.size(); i++) {
			vec_thread[i]->join();
			delete vec_thread[i];
		}

		printf("[time] %f\n", (clock() - current_tick) / (double)CLOCKS_PER_SEC);
	
		Sleep(3000);
	}

	// 윈속 종료
	WSACleanup();
	return 0;
}