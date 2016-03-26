#include "potato.h"
#include <hp_common_defs.h>
#include <stdio.h>
#include <sstream>

Potato::Potato()
{}

Potato::Potato(int aTtl):mTtl(aTtl)
{}

int Potato::getTtl()
{
	return mTtl;
}

std::string Potato::toString()
{
	//can use json for serialization
	//but we know our strings are integers
	//so we use space separated integers
	std::stringstream ss;
	ss << mTtl << " ";
	ss << mPathList.size();
	for(PLAYER_LIST_t::iterator it = mPathList.begin();
			it != mPathList.end(); it++)
	{
		ss << " ";
		ss << *it;
	}
	printd("Potato::toString() <%s>\n", ss.str().c_str());
	return ss.str();
}

void Potato::fromString(std::string& s)
{
	std::stringstream ss;
	ss << s;
	ss >> mTtl;
	int path_list_size = 0;
	ss >> path_list_size;
	printd("Potato::fromString() mTtl<%d> path_list_size<%d>\n", mTtl, path_list_size);
	for(int i = 0; i<path_list_size; i++)
	{
		int player_id = -1;
		ss >> player_id;
		mPathList.push_back(player_id);
		printd("Potato::fromString() player_id<%d>\n", player_id);
	}
}

bool Potato::forward(Socket& s)
{
	std::string serialized = toString();
	if(s.writeString(serialized) == -1)
		return false;
	return true;
}

bool Potato::receive(Socket& s)
{
	std::string serialized = "";
	if(s.readString(serialized) == -1)
		return false;
	fromString(serialized);
	return true;
}

void Potato::printTrace()
{
	printf("Trace of potato:\n");
	for(PLAYER_LIST_t::iterator it = mPathList.begin(); it != mPathList.end() ; )
	{
		printf("%d", *it);
		it++;
		if(it == mPathList.end())
			break;
		printf(",");
	}
	fflush(stdout);
}

void Potato::addToPathList(int aPlayerId)
{
	mPathList.push_back(aPlayerId);
}

void Potato::decrementTtl()
{
	mTtl--;
}
