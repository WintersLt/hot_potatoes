#ifndef __rm_h__
#define __rm_h__

#include "comm.h"

typedef std::map<int, Socket> FD_SOCKET_MAP_t;
typedef std::map<int, std::string> PLAYER_ADD_MAP_t;

class RingMaster
{
	public:
	RingMaster(int aPort, int aNumPlayers, int aHops);

	bool init();
	void registerPlayers();
	void sendNeighbourIds();

	private:
	ServerSocket mRmServer;
	int mPort;
	int mNumPlayers;
	int mHops;
	FD_SOCKET_MAP_t mPlayerSockMap; //store read write socket against player id
	PLAYER_ADD_MAP_t mPlayerAddMap;
};

#endif
