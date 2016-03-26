#ifndef __player_h__
#define __player_h__

#include "comm.h"

typedef std::map<int, Socket> FD_SOCKET_MAP_t;

class Player
{
	public:
	Player(std::string aMasterHostName, int aMasterPort);

	bool init();
	void registerToRm();

	private:
	int mPort;
	std::string mMasterHostname;
	int mMasterPort;
	uint32_t mPlayerId;
	ServerSocket mPotatoServer; //listen for potato and term
	FD_SOCKET_MAP_t mSockMap;  //store left right and master sock
};

#endif
