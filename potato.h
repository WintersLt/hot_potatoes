#ifndef __potato_h__
#define __potato_h__

#include "comm.h"
#include <list>

typedef std::list<int> PLAYER_LIST_t;

class Potato
{
	public:
	Potato();
	Potato(int aTtl);
	int getTtl();
	std::string toString();
	void fromString(std::string& s);
	bool forward(Socket& s);
	bool receive(Socket& s);
	void printTrace();
	void addToPathList(int aPlayerID);
	void decrementTtl();

	private:
	int mTtl;
	PLAYER_LIST_t mPathList;
};

#endif
