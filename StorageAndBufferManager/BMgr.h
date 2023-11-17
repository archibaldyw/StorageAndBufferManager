#pragma once

#include "header.h"
#include "DSMgr.h"

class BMgr
{
public:
	BMgr(DSMgr& dsmgr) {
		memset(ftop, -1, sizeof(ftop));
		memset(ptof, NULL, sizeof(ptof));
		lru_head = NULL;
		lru_tail = NULL;
		ds = &dsmgr;
		hit_cnt = 0;
	}
	// Interface functions
	int FixPage(int page_id, int cmd) {
		int frame_id = -1;
		int hash_id = Hash(page_id);
		BCB* bcb = ptof[hash_id];
		while (bcb != NULL) {
			if (bcb->page_id == page_id) {
				frame_id = bcb->frame_id;
				break;
			}
			bcb = bcb->nextBCB;
		}
		if (frame_id == -1) {	// page not in buffer
			frame_id = SelectVictim();	// choose a frame to replace
			if (ftop[frame_id] != -1) {	// frame is occupied			
				BCB tmp = RemoveBCB(ftop[frame_id]);
				if (tmp.dirty == 1) {	// write back if dirty
					ds->WritePage(tmp.page_id, buf[tmp.frame_id]);
				}
			}
			buf[frame_id] = ds->ReadPage(page_id);
		}
		else {	// page in buffer
			RemoveBCB(page_id);
			hit_cnt++;
		}
		// insert into LRU
		BCB* newbcb = new BCB(page_id, frame_id, 0, 0, ptof[hash_id], lru_head, NULL);
		newbcb->count++;
		lru_head = newbcb;
		if (lru_tail == NULL) {	// initiate lru at first time
			lru_tail = newbcb;
		}
		if (cmd == 1) {	// if write, set dirty
			newbcb->dirty = 1;
		}
		// update hash table
		ptof[hash_id] = newbcb;
		ftop[frame_id] = page_id;
		return frame_id;
	}
	Page FixNewPage() {
		Page page;
		for (int i = 0; i < MAXPAGES; i++) {
			if (!ds->GetUse(i)) {
				ds->SetUse(i, 1);
				page.page_id = i;
				break;
			}
		}
		int page_id = page.page_id;
		int hash_id = Hash(page_id);
		int frame_id = SelectVictim();
		if (ftop[frame_id] != -1) {	// frame is occupied			
			BCB tmp = RemoveBCB(ftop[frame_id]);
			if (tmp.dirty == 1) {	// write back if dirty
				ds->WritePage(tmp.page_id, buf[tmp.frame_id]);
			}
		}
		page.frame_id = frame_id;
		BCB* newbcb = new BCB(page_id, frame_id, 0, 0, NULL, lru_head, NULL);
		lru_head = newbcb;
		if (lru_tail == NULL) {	// initiate lru at first time
			lru_tail = newbcb;
		}
		// update hash table
		ptof[hash_id] = newbcb;
		ftop[frame_id] = page_id;
		return page;
	}
	int UnfixPage(int page_id) {
		int hash_id = Hash(page_id);
		BCB* bcb = ptof[hash_id];
		while (bcb != NULL) {
			if (bcb->page_id == page_id) {
				break;
			}
			bcb = bcb->nextBCB;
		}
		bcb->count--;
		return bcb->count;
	}
	int NumFreeFrames() {
		int cnt = 0;
		for (int i = 0; i < DEFBUFSIZE; i++) {
			if (ftop[i] == -1) {
				cnt++;
			}
		}
		return cnt;
	}
	// Internal Functions
	int SelectVictim() {
		int frame_id = -1;
		if (NumFreeFrames() > 0) {
			for (int i = 0; i < DEFBUFSIZE; i++) {
				if (ftop[i] == -1) {
					frame_id = i;
					break;
				}
			}
		}
		else {
			frame_id = SelectLRUVictim();
		}
		return frame_id;
	}
	int SelectLRUVictim() {
		BCB* tmp = lru_tail;
		while (tmp->count > 0) {
			tmp = tmp->prevLRU;
		}
		return tmp->frame_id;
	}
	int Hash(int page_id) {
		return page_id % DEFBUFSIZE;
	}
	BCB RemoveBCB(int page_id) {
		int hash_id = Hash(page_id);
		BCB* tmp = new BCB();
		BCB* bcb = ptof[hash_id];
		if (bcb->page_id == page_id) {
			tmp = bcb;
			bcb = bcb->nextBCB;
		}
		else {
			while (bcb->nextBCB != NULL) {
				if (bcb->nextBCB->page_id == page_id) {
					tmp = bcb->nextBCB;
					bcb->nextBCB = tmp->nextBCB;
					break;
				}
				bcb = bcb->nextBCB;
			}
		}
		if (tmp->nextLRU != NULL) {
			tmp->nextLRU->prevLRU = tmp->prevLRU;
		}
		else {	// the last one of LRU
			lru_tail = tmp->prevLRU;
		}
		if (tmp->prevLRU != NULL) {
			tmp->prevLRU->nextLRU = tmp->nextLRU;
		}
		else {	// the first one of LRU
			lru_head = tmp->nextLRU;
		}
		ptof[hash_id] = bcb;
		BCB ret = *tmp;
		delete tmp;
		return ret;
	}
	void SetDirty(int frame_id) {
		int page_id = ftop[frame_id];
		BCB *bcb = ptof[Hash(page_id)];
		while (bcb != NULL)
		{
			if (bcb->page_id == page_id)
			{
				bcb->dirty = 1;
				break;
			}
			bcb = bcb->nextBCB;
		}
	}
	void UnsetDirty(int frame_id) {
		int page_id = ftop[frame_id];
		BCB *bcb = ptof[Hash(page_id)];
		while (bcb != NULL)
		{
			if (bcb->page_id == page_id)
			{
				bcb->dirty = 0;
				break;
			}
			bcb = bcb->nextBCB;
		}
	}
	void WriteDirtys() {
		BCB* bcb = NULL;
		for (int i = 0; i < DEFBUFSIZE; i++) {
			bcb = ptof[i];
			while (bcb != NULL) {
				if (bcb->dirty == 1) {
					ds->WritePage(bcb->page_id, buf[bcb->frame_id]);
				}
				bcb = bcb->nextBCB;
			}
		}
	}
	void WriteBuffer(int frame_id, bFrame& frm) {
		buf[frame_id] = frm;
	}
	void PrintFrame(int frame_id) {
		cout<<buf[frame_id].field<<endl;
	}
public:
	int hit_cnt;			// hit count
private:
	// Hash Table
	int ftop[DEFBUFSIZE];	// frame to page
	BCB* ptof[DEFBUFSIZE];	// page to frame
	DSMgr* ds;				// data storage manager
	bFrame buf[DEFBUFSIZE];	// buffer pool
	BCB* lru_head;			// recently used
	BCB* lru_tail;			// least recently used		
};

