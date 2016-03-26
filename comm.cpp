#include "hp_common_defs.h"
#include "comm.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

//Sockt class definitions

int Socket::read(int count, const char* buf)
{
    int bytes_read = 0;
    int bytes_to_read = count;
    int len = 0;
    while(bytes_read != count)
    {
        len = ::read(mFd, (void*)(buf + bytes_read), bytes_to_read);
		if(len == -1)
		{
			printe("Socket::read<%d>: read <%d> bytes out of <%d>\n", mFd, 
					bytes_read, count);
			perror("read error");
			return bytes_read;
		}
		
        bytes_read += len;
        bytes_to_read -= len;
        printd("_read_bytes: bytes_read<%d> bytes_to_read<%d>\n", bytes_read, bytes_to_read);
    }		

	return count;
}

int Socket::write(int count, const char* buf)
{
	int retval = -1;
	if((retval = ::write(mFd, buf, count)) == -1)
		perror("write error");
	return retval;
}

int Socket::readUint32(uint32_t& i)
{
	int retval = -1;
	i = 0;
    retval = read(sizeof(i), (char*)&i);                                            
    i = ntohl(i);                                                            
    printd("Socket::readUint32() read integer<%u>\n", i); 
	return retval;
}

int Socket::writeUint32(uint32_t i)
{
	int retval = -1;
	uint32_t i_to_send = htonl(i);
	retval = write(sizeof(i_to_send), (char*)&i_to_send);
	return retval;
}

int Socket::writeString(std::string& s)
{
	int retval = -1;
	uint32_t str_len = s.size();
	retval = writeUint32(str_len);
	if(retval == -1)
		return retval;
	retval = this->write(str_len, s.c_str());
	printd("Socket::writeString() wrote <%d> bytes\n", retval);
	return retval;
}

int Socket::readString(std::string& s)
{
	int retval = -1;
	uint32_t str_len = 0;
	retval = readUint32(str_len);
	printd("Socket::readString() about to read <%u> bytes\n", str_len);
	if(retval == -1 || str_len == 0)
		return retval;
	char* buf = new char[str_len + 1]();
	retval = read(str_len, buf);
	s = buf;
	delete[] buf;
	return retval;
}
////////////////////////////////////////
ServerSocket::ServerSocket():mPort(0)
{}

ServerSocket::ServerSocket(int aPort):mPort(aPort)
{}

uint16_t ServerSocket::getPort()
{
	return mPort;
}

bool ServerSocket::start_server()
{
	int s;
	int on = 1;
	sockaddr_in saddr;

	memset (&saddr, 0, sizeof(sockaddr_in));

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);
	saddr.sin_port = htons(mPort);

	if ((s = socket (AF_INET, SOCK_STREAM, AF_UNSPEC)) < 0) 
	{
		printe("ServerSocket::start_server: failed to open socket\n");
		perror ("socket error");
		return false;
	}

	/*
	 * socket port doesn't linger after close
	 */
	if (setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on)) < 0) 
	{
		printe("ServerSocket::start_server: failed to set socket options\n");
		perror ("setsockopt reuse error");
		close(s);
		return false;
	}

	//bind socket to ip port
	if (bind (s, (sockaddr *)&saddr, sizeof(sockaddr))) 
	{
		printe("ServerSocket::start_server: failed to bind socket\n");
		perror("bind error");
		close(s);
		return false;
	}

	//To get the port I am listening on
	if (mPort == 0) 
	{
		sockaddr_in name;
		memset (&name, 0, sizeof(sockaddr_in));
		socklen_t len = sizeof(name);
		if (getsockname(s, (sockaddr *) &name, &len) < 0) {
			perror("getsockname error");
			close(s);
			return(0);
		}
		mPort = ntohs(name.sin_port);
	}

	//start listening
	if (listen(s, 128)) 
	{
		perror("listen error");
		close(s);
		return false;
	}

	mListenSock = s;

	return true;
}


int ServerSocket::accept_conn(std::string& aSrcIpPort)
{
	int client_sock_fd, port;	
	char ipstr[INET6_ADDRSTRLEN + 1];
	sockaddr_storage saddr = {};
	socklen_t len = sizeof(saddr);
	while ((client_sock_fd = accept(mListenSock, (struct sockaddr*)&saddr, &len)) < 0)
	{
		if (errno != EINTR) 
		{
			perror("accept error");
			return -1;
		}
	}
	if (saddr.ss_family == AF_INET) 
	{
		sockaddr_in *in_saddr = (sockaddr_in *)&saddr;
		port = ntohs(in_saddr->sin_port);
		inet_ntop(AF_INET, &in_saddr->sin_addr, ipstr, sizeof(ipstr));
		std::stringstream ss;
		ss << ipstr << ":" << port;
		aSrcIpPort = ss.str();
	} 
	else 
	{
		printe("ServerSocket::accept_conn() IPV6 addresses not supported\n");
	}
	return client_sock_fd;
}

///////////////////////////////////////////////////

ClientSocket::ClientSocket(std::string& aDestHost, int aDestPort):mDestHost(aDestHost), 
																  mDestPort(aDestPort)
{}

bool ClientSocket::connect_to_server()
{
	sockaddr_in saddr = {};
	hostent *hp;
	if (!(hp = gethostbyname(mDestHost.c_str()))) 
	{
		perror("gethostbyname");
		return false;
	}

	saddr.sin_addr = *(in_addr *) *hp->h_addr_list;

	saddr.sin_family = hp->h_addrtype;
	saddr.sin_port = htons(mDestPort);

	int s;
	if ((s = socket (hp->h_addrtype, SOCK_STREAM, AF_UNSPEC)) < 0) 
	{
		perror ("socket error");
		return false;
	}

	unsigned int num_retries = 0;
	while (connect(s, (sockaddr *) &saddr, sizeof(sockaddr)) != 0
			&& num_retries <= MAX_RETRIES)
	{
		if (errno != EINTR) 
		{
			perror("connect error");
			close(s);
			return false;
		}
		num_retries++;
		sleep(RETRY_INTERVAL_SEC);
	}
	setFd(s);
	return true;
}

