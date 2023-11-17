#pragma once

#include "header.h"

class DSMgr
{
public:
	DSMgr() {
		currFile = NULL;
		int i = 0;
		iCnt = 0;
		oCnt = 0;
		numPages = 0;
		memset(pages, 0, 100000 * sizeof(bool));
	}
	~DSMgr() {
		if (currFile != NULL) {
			fclose(currFile);
		}
	}
	int OpenFile(string filename) {
		currFile = fopen(filename.c_str(), "r+");
		if (currFile == NULL) {
			currFile = fopen(filename.c_str(), "w+");
			if (currFile == NULL) {
				return 0;
			}
		}
		fseek(currFile, 0, SEEK_END);
		long fileSize = ftell(currFile);
		int tmp_numPages = numPages;
		numPages += (fileSize + PAGESIZE - 1) / PAGESIZE;	// round up
		memset(&pages[tmp_numPages], 1, numPages - tmp_numPages);	// set use bits
		return 1;
	}
	int CloseFile() {
		if (currFile == NULL) {
			return 0;
		}
		fclose(currFile);
		currFile = NULL;
		return 1;
	}
	// read a page from the file
	bFrame ReadPage(int page_id) {
		bFrame frm;
		if (currFile == NULL) {
			return frm;
		}
		fseek(currFile, page_id * PAGESIZE, SEEK_SET);
		fread(frm.field, sizeof(char), PAGESIZE, currFile);
		iCnt++;
		return frm;
	}
	// write a page to the file
	int WritePage(int page_id, bFrame& frm) {
		if (currFile == NULL) {
			return 0;
		}
		fseek(currFile, page_id * PAGESIZE, SEEK_SET);
		fwrite(frm.field, sizeof(char), PAGESIZE, currFile);
		oCnt++;
		return 1;
	}
	// seek to the page_id
	int Seek(int offset, int pos) {
		if (currFile == NULL) {
			return 0;
		}
		fseek(currFile, offset, pos);
		return 1;
	}
	FILE* GetFile() {
		return currFile;
	}
	void IncNumPages() {
		numPages++;
	}
	int GetNumPages() {
		return numPages;
	}
	void SetUse(int index, int use_bit) {
		pages[index] = use_bit;
	}
	int GetUse(int index) {
		return pages[index];
	}
public:
	int iCnt;
	int oCnt;
private:
	FILE* currFile;
	int numPages;	// number of pages in the file
	bool pages[MAXPAGES];	// array of use bits
};

