#include "comm.h"
#include "player.h"
#include "hp_common_defs.h"
#include "arpa/inet.h"
#include <sstream>
#include <unistd.h>

Player::Player(std::string aMasterHostName, int aMasterPort):mPort(0),
	mMasterHostname(aMasterHostName), mMasterPort(aMasterPort), mPlayerId(0)
{}

bool Player::init()
{
	//start listening on any available port
	//register to rm and send it your hostname and port
	//recieve player id from master
	//save  master socket fd in map
	//block until you recive right neighbor's address from master
	//connect to right neighbour
	//accept connection from left neighbour
	//return true
	//enter the potato loop
	if(!mPotatoServer.start_server())
	{
		printe("Not able to start server on port<%d>\n", mPort);
		return false;
	}
	//connect to master
	ClientSocket master_cs(mMasterHostname, mMasterPort);
	if(!master_cs.connect_to_server())
	{
		printe("Not able to connect to master\n");
		return false;
	}
	
	//TODO Object slicing, use pointers instead
	Socket s = (Socket)master_cs;
	mSockMap[0] = s; 

	//recieve player id
	mPlayerId = 0;
	if(s.readUint32(mPlayerId) == -1)
	{
		printe("Player::init() readUint32 failed\n");
		return false;
	}
	printd("Player::init() received player id<%d>\n", mPlayerId);

	//send my listen port to master
	s.writeUint32(mPotatoServer.getPort());

	//read right neighbour add
	std::string right_node_add = "";
	if(s.readString(right_node_add) == -1)
	{
		printe("Player::init() readString failed\n");
		return false;
	}
	printd("Player::init() received right neighbour add<%s>\n", right_node_add.c_str());
	//split into hostname and port
	uint16_t rn_port = 0;
	size_t colon_pos = right_node_add.find_last_of(":");
	std::string rn_hostname = right_node_add.substr(0, colon_pos);
	std::string rn_port_str = right_node_add.substr(colon_pos+1);
	std::stringstream ss;
	ss << rn_port_str;
	ss >> rn_port;
	printd("Player::init() right neighbour hostname<%s> port<%u>\n", rn_hostname.c_str(), 
			rn_port);

	//connect to right neighbour
	ClientSocket rn_cs(rn_hostname, rn_port);
	if(!rn_cs.connect_to_server())
	{
		printe("Not able to connect to right neighbour\n");
		return false;
	}
	mSockMap[1] = (Socket)rn_cs; 
	
	//accept connection from left neighbour
	std::string ln_ip_port = "";
	int ln_fd = mPotatoServer.accept_conn(ln_ip_port);
	if(ln_fd == -1)
	{
		printe("Not able to connect to left neighbour\n");
		return false;
	}
	Socket ln_s(ln_fd);
	mSockMap[2] = ln_s;
	printd("Player::init() Everything done\n");
	return true;
}

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		printe("Two args expected. <master-machine-name> <port-number>\n");
		exit(1);
	}
	
	std::stringstream ss;
	ss << argv[1] << " " <<  argv[2];

	int master_port = 0; 
	std::string master_hostname;

	ss >> master_hostname >> master_port;
	
	printd("initializing player with master port<%d>, master_hostname<%s>\n", master_port,
			master_hostname.c_str());
	Player player(master_hostname, master_port);

	if(!player.init())
	{
		printe("player init failed\n");
		exit(1);
	}
	
	printd("all done\n");
}
