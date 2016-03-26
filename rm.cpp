/*
 * read cmd line
 * Open server socket
 * start accepting connection
 * reply to the guy with player id
 * map the socket fd to player id
 * once all players have registered, create and send token to one of the nodes
 * check the ttl before sending token
 * put all the sockets for select/epoll
 * once a read event is recieved from one of the sockets
 * read the token, and print its contents
 * send signal to every player to shutdown
 * shutdown self
 */


#include "comm.h"
#include "rm.h"
#include "hp_common_defs.h"
#include "arpa/inet.h"
#include <sstream>
#include <unistd.h>

RingMaster::RingMaster(int aPort, int aNumPlayers, int aHops): mRmServer(aPort),
	mPort(aPort), mNumPlayers(aNumPlayers), mHops(aHops)
{}

bool RingMaster::init()
{
	if(!mRmServer.start_server())
	{
		printe("Not able to start server on port<%d>\n", mPort);
		return false;
	}
	return true;
}

void RingMaster::registerPlayers()
{
	int num_registered_players = 0;

	while(num_registered_players < mNumPlayers)
	{
		std::string srcIpPort = "";
		int client = mRmServer.accept_conn(srcIpPort);
		if(client != -1)
		{
			Socket s(client);
			mPlayerSockMap[num_registered_players] = s;
			uint32_t player_id = htonl(num_registered_players);
			printd("RingMaster::registerPlayers: sending player_id<%d> to client<%s>\n",
					num_registered_players, srcIpPort.c_str());
			s.write(sizeof(player_id), (char*)&player_id);

			//read listen port number
			uint32_t listen_port = 0;
			s.readUint32(listen_port);

			//store player hostname and port
			std::string playeradd = srcIpPort.substr(0, srcIpPort.find(":"));
			std::stringstream ss;
			ss << playeradd << ":" << listen_port;
			mPlayerAddMap[num_registered_players] = ss.str();
			printd("RingMaster::registerPlayers() adding player <%s>\n", 
					mPlayerAddMap[num_registered_players].c_str());

			num_registered_players++;
		}
		else
			printd("RingMaster::registerPlayers() accept_conn() returned -1\n");
	}
}

void RingMaster::sendNeighbourIds()
{
	int left_id = mNumPlayers -1; //node left to curr player_id
	for(int player_id = 0; player_id < mNumPlayers; player_id++)
	{
		mPlayerSockMap[left_id].writeString(mPlayerAddMap[player_id]);
		printd("RingMaster::sendNeighbourIds():sending <%s> to <%d> sock_fd<%d>\n", 
				mPlayerAddMap[player_id].c_str(), left_id, mPlayerSockMap[left_id].getFd());
		left_id = (left_id + 1) % mNumPlayers;
	}
}

int main(int argc, char* argv[])
{
	if(argc < 4)
	{
		printe("Three args expected. <port-number> <number-of-players> <hops>\n");
		exit(1);
	}
	
	std::stringstream ss;
	ss << argv[1] << " " <<  argv[2] << " " << argv[3];

	int port = 0, num_players = 0, hops = 0; 

	ss >> port >> num_players >> hops;
	
	printd("starting master with port<%d>, num_players<%d> hops<%d>\n", port,
			num_players, hops);
	RingMaster rm(port, num_players, hops);

	if(!rm.init())
	{
		printe("init failed\n");
		exit(1);
	}
	
	rm.registerPlayers();
	
	//all players registered
	//now send neighbor ids to each node
	
	rm.sendNeighbourIds();

	sleep(100);
	printd("all done\n");
}
