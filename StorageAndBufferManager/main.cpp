#include "BMgr.h"

void generateData() {
	FILE* fp = fopen("data.dbf", "w+");
	char bu[50000];
	memset(bu, 'a', sizeof(bu));	
	for (int i = 0; i < 4096; i++)
	{
		fwrite(bu, sizeof(char), sizeof(bu), fp);
	}
	fclose(fp);
}

int main()
{
	//generateData();	// page 0~49999
	// 'a': raw data
	// 'b': modified data
	// 'c': new data
	
	// init
	DSMgr* dsmgr = new DSMgr;
	dsmgr->OpenFile("data.dbf");
	BMgr* bmgr = new BMgr(*dsmgr);

	/* ------------------write a new page------------------
	
	Page page = bmgr->FixNewPage();
	bmgr->FixPage(page.page_id, 1);
	bFrame frm;
	memset(frm.field, 'c', sizeof(frm.field));	
	bmgr->WriteBuffer(page.frame_id, frm);
	bmgr->UnfixPage(page.page_id);

	-----if succeed, cmd 1,50000 will read this page----- */
	
	// read cmd
	ifstream infile("data-5w-50w-zipf.txt");
	int x = 0;
	int y = -1;
	string line;
	string token;

	// ------------------------ time ------------------------
	// ------------------------------------------------------
	auto beginTime = chrono::high_resolution_clock::now();
	while (getline(infile, line)) {
		istringstream iss(line);
		if (std::getline(iss, token, ',')) {
			x = std::stoi(token);
		}
		if (std::getline(iss, token, ',')) {
			y = std::stoi(token);
		}
		int tmp_frame_id = bmgr->FixPage(y, x);
		/*
			read or write page
			x=0: read
			x=1: write
			y: page_id
		*/

		/* -----------------------------------------------
		if (x == 1) {
			bFrame frm;
			memset(frm.field, 'b', sizeof(frm.field));	
			bmgr->WriteBuffer(tmp_frame_id, frm);
		}
		-------------------------------------------------*/
		bmgr->UnfixPage(y);
	}
	bmgr->WriteDirtys();
	auto endTime = chrono::high_resolution_clock::now();
	// ------------------------------------------------------
	// ------------------------ time ------------------------

	auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - beginTime);
	cout << "Max Pages: " << MAXPAGES << " page" << endl;
	cout << "Frame Size: " << FRAMESIZE << " B" << endl;
	cout << "Page Size: " << PAGESIZE << " B" << endl;
	cout << "Buffer Size: " << DEFBUFSIZE << " page" << endl;
	cout << endl;
	cout << "Time: " << duration.count() << " ms" << endl;
	cout << "Read: " << dsmgr->iCnt << " times" << endl;
	cout << "Write: " << dsmgr->oCnt << " times" << endl;
	cout << "Hit: " << bmgr->hit_cnt << " times" << endl;
	cout << "Hit Rate:" << bmgr->hit_cnt / 5000 << "%" << endl;
}