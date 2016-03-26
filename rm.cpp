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

// TODO 
// Complete player side poll
//   Recive potato, decrement hop count, check if zero
//   append self to pathlist, forward
// Also send the total number of players to each player, helful for printing left aand right neighbour
// Add prints as per specs
// read spec again
// Run on VCL once

#include "comm.h"
#include "rm.h"
#include "hp_common_defs.h"
#include "arpa/inet.h"
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <potato.h>
#include <poll.h>
#include <netdb.h>
#include <string.h>

RingMaster::RingMaster(int aPort, int aNumPlayers, int aHops): mRmServer(aPort),
	mPort(aPort), mNumPlayers(aNumPlayers), mHops(aHops)
{}

bool RingMaster::init()
{
	//seeding random num generator
	srand(mNumPlayers);
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
			
			//send num players
			s.writeUint32(mNumPlayers);

			//read listen port number
			uint32_t listen_port = 0;
			s.readUint32(listen_port);

			//store player hostname and port
			std::string srcIp = srcIpPort.substr(0, srcIpPort.find(":"));


			std::string playeradd = srcIp;
			std::stringstream ss;
			ss << playeradd << ":" << listen_port;
			mPlayerAddMap[num_registered_players] = ss.str();
			printd("RingMaster::registerPlayers() adding player <%s>\n", 
					mPlayerAddMap[num_registered_players].c_str());
			std::string remote_hname = getHostName(srcIp);
			printf("player %d is on %s\n", num_registered_players, remote_hname.c_str());
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

void RingMaster::sendPotato()
{
	int dest_player = rand() % mNumPlayers;	
	printf("All players present, sending potato to player %d\n", dest_player);
	Potato p(mHops);
	if(p.getTtl() == 0)
	{
		printd("RingMaster::sendPotato() numHops is 0, terminating all players\n");
		p.printTrace();
		terminateAllPlayers();
		//TODO cleanup
		exit(0);
	}

	//Protocol is 
	//Command<int>Potato<serialized string>
	mPlayerSockMap[dest_player].writeUint32(CMD_POTATO);
	p.forward(mPlayerSockMap[dest_player]);
}

void RingMaster::waitForPotato()
{
	//poll on all the player sockets here
	//as soon as something is recieved
	//recive potato, print potato, terminate clients
	//exit
	pollfd* pfds = new pollfd[mNumPlayers]();
	//set the fds to be polled
	for(int player_id = 0; player_id < mNumPlayers; player_id++)
	{
		pfds[player_id].fd = mPlayerSockMap[player_id].getFd();
		pfds[player_id].events = POLLIN;
		pfds[player_id].revents = 0;
	}
	int retval = poll(pfds, (unsigned int)mNumPlayers, -1);
	if(retval == -1)
	{
		printd("RingMaster::waitForPotato() poll failed\n");
		return;
	}
	printd("RingMaster::waitForPotato() <%d> sockets readable\n", retval);
	for(int player_id = 0; player_id < mNumPlayers; player_id++)
	{
		if(pfds[player_id].revents & POLLIN)
		{
			//fetch potato
			Potato p;
			p.receive(mPlayerSockMap[player_id]);
			p.printTrace();
			break;
		}
	}
	delete[] pfds;
	printd("RingMaster::waitForPotato() Potato back at ring master, terminating\n");
	terminateAllPlayers();
}

void RingMaster::terminateAllPlayers()
{
	for(int player_id = 0; player_id < mNumPlayers; player_id++)
	{
		mPlayerSockMap[player_id].writeUint32(CMD_TERMINATE);
		mPlayerSockMap[player_id].close();
	}
	printd("RingMaster::terminateAllPlayers() all clients closed, closing server\n");
	mRmServer.close();
}

std::string RingMaster::getHostName(std::string& ip)
{
	sockaddr_in saddr;

	memset (&saddr, 0, sizeof(sockaddr_in));

	hostent *hp;
	saddr.sin_family = AF_INET;
	if (!(hp = gethostbyname(ip.c_str()))) 
	{
		perror("gethostbyname");
		return "";
	}

	saddr.sin_addr = *(in_addr *) *hp->h_addr_list;

	saddr.sin_family = hp->h_addrtype;

	socklen_t len = sizeof(sockaddr_in); 

	char hbuf[NI_MAXHOST] = {};

	if (getnameinfo((const sockaddr*)&saddr, len, hbuf, sizeof(hbuf), NULL,
				0, 0) == 0)
	{
		printd("RingMaster::gethostname() host=%s ip = %s\n", hbuf, ip.c_str());	
		return std::string(hbuf);
	}
	else
		printe("RingMaster::gethostname() getnameinfo failed\n");	

	return "";
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
	char host[256] = {};
	gethostname(host, 255);
	printf("Potato Master on %s\n", host);
	printf("Players = %d\n", num_players);
	printf("Hops = %d\n", hops);
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

	rm.sendPotato();
	
	rm.waitForPotato();

	fflush(stdout);
	printd("all done\n");
}
