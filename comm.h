#ifndef __comm_h__
#define __comm_h__

#include <string>
#include <set>
#include <map>
#include <hp_common_defs.h>

class Socket
{
	public:
	Socket()
	{}
	Socket(int aFd):mFd(aFd)
	{}

	/*
	 * Return num bytes read or -1
	 * same as read system call
	 */
	int read(int count, const char*  buf);

	/*
	 * Return val same as write
	 * Writes to fd, count bytes from buf
	 */
	int write(int count, const char*  buf);

	/*
	 * Return val same as read
	 * First reads size of string,
	 * then reads that many bytes from socket
	 */
	int readString(std::string& s);

	/*
	 * Return val same as write
	 * First writes size of string,
	 * then writes the string on the socket
	 */
	int writeString(std::string& s);

	/*
	 * Return val same as read
	 * reads a 32 bit unsigned
	 */
	int readUint32(uint32_t& i);

	/*
	 * Return val same as write
	 * writes a 32 bit unsigned
	 */
	int writeUint32(uint32_t i);


	void setFd(int aFd)
	{
		mFd = aFd;
	}

	int getFd()
	{
		return mFd;
	}
	private:
	int mFd;
};

/////////////////////////////////////////////////


class ServerSocket : public Socket
{
	public:
	ServerSocket(int aPort);
	ServerSocket();
	
	/*
	 * Bind to port
	 * Start listening
	 */
	bool start_server();

	/*
	 * Accepts a connection and returns its fd
	 * Also adds the client to mClients
	 */
	int accept_conn(std::string& aSrcIpPort);
	
	uint16_t getPort();

	private:
	uint16_t mPort;
	int mListenSock;
};

class ClientSocket: public Socket
{
	public:
	ClientSocket(std::string& aDestHost, int aDestPort);
	
	/*
	 * Returns true if connection successful
	 */
	bool connect_to_server();
	
	private:
	std::string mDestHost;
	int mDestPort;
};

//TODO something for epoll
#endif

