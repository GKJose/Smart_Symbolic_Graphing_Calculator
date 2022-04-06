#ifndef WebSocket_HXX
#define WebSocket_HXX

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
using namespace std;

class WebSocket{

	int socket_desc;
	struct sockaddr_in server;
	bool connected;
	public:
		WebSocket(char* ip,int port);
		bool connectToServer();
		bool sendMessage(char* msg);	
		char* recieveMessage();
		bool isConnected();
};
#endif