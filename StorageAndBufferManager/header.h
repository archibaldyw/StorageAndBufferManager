#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define MAXPAGES 100000
#define FRAMESIZE 4096
#define PAGESIZE 4096
#define DEFBUFSIZE 1024

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <chrono>
using namespace std;

struct bFrame
{
	char field[FRAMESIZE];
};

struct BCB
{
	int page_id;
	int frame_id;
	int count;		// number of times the page is accessed (incomplete)
	int dirty;
	BCB* nextBCB;
	BCB* nextLRU;
	BCB* prevLRU;
	BCB() {
		page_id = -1;
		frame_id = -1;
		count = 0;
		dirty = 0;
		nextBCB = NULL;
		nextLRU = NULL;
		prevLRU = NULL;
	}
	BCB(int page_id, int frame_id, int count, int dirty, BCB* nextBCB, BCB* nextLRU, BCB* prevLRU) {
		this->page_id = page_id;
		this->frame_id = frame_id;
		this->count = count;
		this->dirty = dirty;
		this->nextBCB = nextBCB;
		this->nextLRU = nextLRU;
		this->prevLRU = prevLRU;
	}
};

struct Page
{
	int page_id;
	int frame_id;
	Page() {
		page_id = -1;
		frame_id = -1;
	}
};
