#ifndef NWSVR_H 
#define NWSVR_H

const unsigned short PORT_NUMBER = 5555;
const int MAX_MESSAGE_SIZE = 512;
const int MAX_QUEUE_SIZE = 200;
const int MAX_CLIENTS = 1000;

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <mmsystem.h>
#include <iostream>
#include <thread>
#include <stdio.h>
#include <stdlib.h> // rand
#include <time.h> // 
//#include <windows.h>

#include "NWMessages.h"
#include "GameWorld.h"

class NWSvr
{
private:
	struct QueueT
	{
		bool isActive;
		struct sockaddr_in address;
		int size;
		char message[MAX_MESSAGE_SIZE];
	};

	struct SpawnInfo
	{
		float x, y, z;
		bool inUse;
	};

	struct ClientT
	{
		bool online;
		unsigned short sessionID;
		struct sockaddr_in clientAddress;
	};

public:
	NWSvr();
	NWSvr(const NWSvr& other){}
	~NWSvr();

	bool Init(float tickRate);
	bool InitWinSock();
	void Update();

	bool Online() {return m_Online;}
	SOCKET GetServerSocket(){ return m_Socket;}
	void SetWorldInfo(GameWorld* world);
	void AddMSGToQ(char*, int, struct sockaddr_in);
	float GetTickRate(){return m_tickRate;}

	void Pete(){cout << "Tick...\n";}


private:

	bool InitServerSocket();
	void ProcessQueue();

	void HandleConnectMessage(struct sockaddr_in);
	void HandlePingMessage(int);
	void HandleDisconnectMessage(int);
	void HandleChatMessage(int);
	void HandleEntityRequestMessage(int);
	void HandleStateChangeMessage(int);
	void HandlePositionMessage(int);

	void GetNextID(unsigned short&, unsigned short&);
	bool VerifyUser(unsigned short, unsigned short, int);

	bool m_Online;
	unsigned long startTime, endTime, s_tickTime, e_tickTime;
	int m_activeClients;
	float m_tickRate;
	SOCKET m_Socket;
	QueueT* m_networkMessageQueue;
	int m_nextQueueLocation, m_nextMessageForProcessing;
	ClientT* m_clientList;
	GameWorld* m_GameWorld;	
	SpawnInfo SpawnPosition[10];
};

#endif