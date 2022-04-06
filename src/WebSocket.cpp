#include <WebSocket.hxx>
#include <stdio.h>
WebSocket::WebSocket(char* ip, int port){
	socket_desc = socket(AF_INET,SOCK_STREAM,PF_INET);
	if(socket_desc == -1) printf("could not create socket!");
	
	if(inet_pton(AF_INET, ip, &server.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
    }
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	connected = false;
}
bool WebSocket::connectToServer(){
	connected = !(connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0);
	printf("connected %s",connected? "yes":"no");
	return connected;
}

bool WebSocket::sendMessage(char* msg){
	return !( send(socket_desc , msg , strlen(msg) , 0) < 0);
}
char* WebSocket::recieveMessage(){
	char* server_reply[2000];
	if (recv(socket_desc, server_reply , 2000 , 0) < 0){
	}
	printf(*server_reply);
	return *server_reply;
}
bool WebSocket::isConnected(){
	return connected;
}