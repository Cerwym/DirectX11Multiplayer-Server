#include "NWSvr.h"

void ServerListenFunc(void* ptr);

NWSvr::NWSvr()
{
	m_networkMessageQueue = 0;
	m_clientList = 0;
	m_GameWorld	= 0;
	
	SpawnPosition[0].x = 0.0;	SpawnPosition[0].y = 0.0; SpawnPosition[0].z = 0.0;
	SpawnPosition[1].x = 10.0;	SpawnPosition[1].y = 0.0; SpawnPosition[1].z = 0.0;
	SpawnPosition[2].x = -10.0; SpawnPosition[2].y = 0.0; SpawnPosition[2].z = 0.0;
	SpawnPosition[3].x = 0.0;	SpawnPosition[3].y = 0.0; SpawnPosition[3].z = 10.0;
	SpawnPosition[4].x = 0.0;	SpawnPosition[4].y = 0.0; SpawnPosition[4].z = -10.0;
	SpawnPosition[5].x = 10.0;	SpawnPosition[5].y = 0.0; SpawnPosition[5].z = -10.0;
	SpawnPosition[6].x = -10.0; SpawnPosition[6].y = 0.0; SpawnPosition[6].z = -10.0;
	SpawnPosition[7].x = 5.0;	SpawnPosition[7].y = 0.0; SpawnPosition[7].z = 20.0;
	SpawnPosition[8].x = 20.0;	SpawnPosition[8].y = 0.0; SpawnPosition[8].z = 5.0;
	SpawnPosition[9].x = 30.0;	SpawnPosition[9].y = 0.0; SpawnPosition[9].z = 5.0;

	for (int i = 0; i < 10; i++)
		SpawnPosition[i].inUse = false;

	srand(time(NULL));
}

NWSvr::~NWSvr()
{
	// Set the server to be offline
	m_Online = false;

	if (closesocket(m_Socket) !=0)
	{
		cout << "Could not close the server socket correctly." << endl;
	}

	if (m_clientList)
	{
		delete [] m_clientList;
		m_clientList = 0;
	}

	if (m_networkMessageQueue)
	{
		delete [] m_networkMessageQueue;
		m_networkMessageQueue = 0;
	}

	m_GameWorld = 0;

	WSACleanup();

}

bool NWSvr::Init(float tickRate)
{
	m_networkMessageQueue = new QueueT[MAX_QUEUE_SIZE];
	if (!m_networkMessageQueue)
		return false;

	// Seed by time
	srand(time(NULL));

	m_nextQueueLocation = 0;
	m_nextMessageForProcessing = 0;

	for (int i = 0; i < MAX_QUEUE_SIZE; i++)
	{
		m_networkMessageQueue[i].isActive = false;
	}

	m_clientList = new ClientT[MAX_CLIENTS];
	if (!m_clientList)
	{
		return false;
	}

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		m_clientList[i].online = false;
	}

	m_tickRate = tickRate;

	if (!InitWinSock())
		return false;

	if (!InitServerSocket())
		return false;

	cout << "Server now set to " << m_tickRate << " updates per second." << endl;
	return true;
}

bool NWSvr::InitWinSock()
{
	WSADATA SockData;
	WSAPROTOCOL_INFOW* protocolBuffer;
	unsigned long bufferSize;
	int protocols[2];

	int error = WSAStartup(0x0202, &SockData);
	if (error != 0)
	{
		// Handle WSAStartup Error
	}

	// Request the buffer size needed for holding the available protocols
	WSAEnumProtocols(NULL, NULL, &bufferSize);

	// Create a buffer for the protocol information structs
	protocolBuffer = new WSAPROTOCOL_INFOW[bufferSize];
	if (!protocolBuffer)
		return false;

	protocols[0] = IPPROTO_TCP;
	protocols[1] = IPPROTO_UDP;

	error = WSAEnumProtocols(protocols, protocolBuffer, &bufferSize);
	if (error == SOCKET_ERROR)
		return false;

	delete[] protocolBuffer;
	protocolBuffer = 0;

	return true;
}

bool NWSvr::InitServerSocket()
{
	int error;
	unsigned long setting;

	m_Online = true;

	// Create a UDP socket
	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_Socket == INVALID_SOCKET)
	{
		cout << "Error: Could not create UDP socket!" << endl;
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NUMBER);
	serverAddr.sin_addr.s_addr = htons(INADDR_ANY);

	if (::bind(m_Socket, (const sockaddr*) &serverAddr, sizeof(serverAddr)) != 0)
	{
		cout << "Error: Could not bind the socket." << endl;
		false;
	}

	setting = 1;
	error = ioctlsocket( m_Socket, FIONBIO, &setting);
	if (error == -1)
	{
		cout << "Error: Could not set socket to non-blocking I/O." << endl;
		return false;
	}

	// Start a thread to listen for incoming messages
	// definitely a problem with the thread

	//std::thread t(&ServerListenFunc, (void*)this);

	std::cout << "Server initialized successfully, listening on port " << PORT_NUMBER << endl;

	m_activeClients = 0;
	startTime = timeGetTime();
	endTime = startTime + 5000;
	s_tickTime = timeGetTime();
	e_tickTime = s_tickTime + (	(m_tickRate / 60 * 1000));
	return true;
}

void NWSvr::SetWorldInfo(GameWorld* world)
{
	m_GameWorld = world;
}

void NWSvr::Update()
{
	int clientLength;
	int bytesRead = 0;
	char recvBuffer[4096];
	struct sockaddr_in clientAddress;


	//if (s_tickTime >= e_tickTime)
	//{
		//cout << "Tick...\n";
		clientLength = sizeof(clientAddress);
		bytesRead = recvfrom(m_Socket, recvBuffer, 4096, 0, (struct sockaddr*)&clientAddress, &clientLength);
		if (bytesRead > 0)
		{
			// A message was received, add it to the queue for further processing.
			AddMSGToQ(recvBuffer, bytesRead, clientAddress);
		}

		//e_tickTime = s_tickTime + (	(m_tickRate / 60 * 1000));
	//}

	// If there are no active clients, post a message every 10 seconds for debug output
	if (m_activeClients == 0 && startTime >= endTime)
	{
		cout << "No active clients..." << endl;
	}
	if (startTime >= endTime)
		endTime = startTime + 5000;

	startTime = timeGetTime();
	s_tickTime = timeGetTime();

	ProcessQueue();
}

void NWSvr::GetNextID(unsigned short& newID, unsigned short& sessionID)
{
	bool done = false;
	int i = 0;

	//  Start at -1, 0 may be a valid ID
	newID = -1;

	while(!done)
	{
		if (m_clientList[i].online == false)
		{
			newID = i;
			done = true;
		}
		else
		{
			i++;
		}

		// Check to see if the maximum number of clients has been reached
		if (i == MAX_CLIENTS)
		{
			done = true;
		}
	}

	// Generate a random session ID for the client
	sessionID = rand() % 65000;

}

void NWSvr::AddMSGToQ(char* message, int messageSize, struct sockaddr_in clientAddress)
{
	char* ipAddress;

	// Check for buffer overflow.
	if(messageSize > MAX_MESSAGE_SIZE)
	{
		ipAddress = inet_ntoa(clientAddress.sin_addr);
		cout << "WARNING-0001: Possible buffer overflow attack from IP: " << ipAddress << endl;

		// Add timestamp to warnings.

	}

	// Otherwise add it to the circular message queue to be processed.
	else
	{
		m_networkMessageQueue[m_nextQueueLocation].address = clientAddress;
		m_networkMessageQueue[m_nextQueueLocation].size = messageSize;
		memcpy(m_networkMessageQueue[m_nextQueueLocation].message, message, messageSize);

		// Set it to be active last so that racing conditions in processing the queue do not exist.
		m_networkMessageQueue[m_nextQueueLocation].isActive = true;

		// Increment the queue position.
		m_nextQueueLocation++;
		if(m_nextQueueLocation == MAX_QUEUE_SIZE)
		{
			m_nextQueueLocation = 0;
		}
	}
}

void NWSvr::ProcessQueue()
{
	MSG_GENERIC_DATA* message;
	char* ipAddress;

	// Loop through and process all the active messages in the queue.
	while(m_networkMessageQueue[m_nextMessageForProcessing].isActive == true)
	{
		// Coerce the message into a generic format to read the type of message.
		message = (MSG_GENERIC_DATA*)m_networkMessageQueue[m_nextMessageForProcessing].message;    

		// Process the message based on the message type.
		switch(message->type)
		{
		case MSG_CONNECT:
			{
				HandleConnectMessage(m_networkMessageQueue[m_nextMessageForProcessing].address);
				break;
			}
		case MSG_PING:
			{
				HandlePingMessage(m_nextMessageForProcessing);
				break;
			}
		case MSG_DISCONNECT:
			{
				HandleDisconnectMessage(m_nextMessageForProcessing);
				break;
			}
		case MSG_CHAT:
			{
				HandleChatMessage(m_nextMessageForProcessing);
				break;
			}
		case MSG_ENTITY_REQUEST:
			{
				HandleEntityRequestMessage(m_nextMessageForProcessing);
				break;
			}
		case MSG_STATE_CHANGE:
			{
				HandleStateChangeMessage(m_nextMessageForProcessing);
				break;
			}
		case MSG_POSITION:
			{
				HandlePositionMessage(m_nextMessageForProcessing);
				break;
			}
		default:
			{
				ipAddress = inet_ntoa(m_networkMessageQueue[m_nextMessageForProcessing].address.sin_addr);
				cout << "WARNING-0002: Received an unknown message type from IP: " << ipAddress << endl;
				break;
			}
		}

		// Set the message as processed.
		m_networkMessageQueue[m_nextMessageForProcessing].isActive = false;

		// Increment the queue position.
		m_nextMessageForProcessing++;

		if(m_nextMessageForProcessing == MAX_QUEUE_SIZE)
		{
			m_nextMessageForProcessing = 0;
		}
	}

	return;
}

void NWSvr::HandleConnectMessage(struct sockaddr_in clientAddress)
{
	unsigned short newId;
	unsigned short sessionId;
	char* ipAddress;
	MSG_NEWID_DATA message;
	int bytesSent, i;
	MSG_ENTITY_INFO_DATA message2;
	MSG_CHAT_DATA infoMSG;
	XMFLOAT3 position, rotation;

	// Get the next free ID number and session ID and then assign it to this client.
	GetNextID(newId, sessionId);
	if(newId != -1)
	{
		ipAddress = inet_ntoa(clientAddress.sin_addr);
		cout << "Received a CONNECT message from " << ipAddress << ".  Assigning ID : " << newId << " Session ID : " << sessionId << "." << endl;

		//
		int index;
		do 
		{
			index = rand() % 10;
		} while (SpawnPosition[index].inUse);
		SpawnPosition[index].inUse = true;

		position.x = SpawnPosition[index].x;
		position.y = SpawnPosition[index].y;
		position.z = SpawnPosition[index].z;

		cout << "New User Spawned at " << position.x  <<"," << position.y << "," << position.z << endl; 

		// Set the user to online and store the client address.
		m_clientList[newId].online = true;
		m_clientList[newId].sessionID = sessionId;
		m_clientList[newId].clientAddress = clientAddress;

		// Create a new id message for the user.
		message.type = MSG_NEWID;
		message.idNumber = newId;
		message.sessionId = sessionId;
		message.position = position;

		// Send the user notification of their ids.
		bytesSent = sendto(m_Socket, (char*)&message, sizeof(MSG_NEWID_DATA), 0, (struct sockaddr*)&clientAddress, sizeof(clientAddress));
		if(bytesSent != sizeof(MSG_NEWID_DATA))
		{
			cout << "WARNING: Error sending new ID message to client with IP: " << ipAddress << endl;
		}

		// Add the new user to the zone.
		m_GameWorld->AddNewUser(newId);
		m_GameWorld->SetData(newId, position, XMFLOAT3(0.0f, 0.0f, 0.0f), timeGetTime());
		m_activeClients++;

		// Notify all other network clients that a new user has logged in.
		message2.type = MSG_NEW_USER_LOGIN;
		message2.entityId = newId;
		message2.entityType = ENTITY_TYPE_USER;

		message2.position = position;
		message2.rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);

		infoMSG.type = MSG_CHAT;
		infoMSG.idNumber = newId;
		strcpy(infoMSG.text, "A new user has connected!\n");

		for(i=0; i<MAX_CLIENTS; i++)
		{
			if(m_clientList[i].online)
			{
				// if the currently iterated player is NOT the new connection
				if(i != newId)
				{
					// send the new information to all other players
					bytesSent = sendto(m_Socket, (char*)&message2, sizeof(MSG_ENTITY_INFO_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
					if(bytesSent != sizeof(MSG_ENTITY_INFO_DATA))
					{
						cout << "WARNING: Error sending new user login message to client." << endl;
					}

					bytesSent = sendto(m_Socket, (char*)&infoMSG, sizeof(MSG_CHAT_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
					if (bytesSent != sizeof(MSG_CHAT_DATA))
					{
						cout << "WARNING: Error sending confirmation chat data." << endl;
					}
				}
			}
		}
	}
	else
	{
		cout << "WARNING: Max clients connected reached!" << endl;
	}

	return;
}

void NWSvr::HandleDisconnectMessage(int queuePosition)
{
	MSG_DISCONNECT_DATA* msg;
	unsigned short clientId, sessionId;
	bool result;
	char* ipAddress;
	MSG_USER_DISCONNECT_DATA message;
	int i, bytesSent;


	// Coerce the message into a generic format to read the type of message.
	msg = (MSG_DISCONNECT_DATA*)m_networkMessageQueue[queuePosition].message;

	// Get the id numbers.
	clientId = msg->idNumber;
	sessionId = msg->sessionId;

	// Verify that the client is legitimate.
	result = VerifyUser(clientId, sessionId, queuePosition);
	if(!result)
	{
		return;
	}

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	cout << "Received DISCONNECT message from " << sessionId << "." << endl;

	// Disconnect the user.
	m_clientList[clientId].online = false;

	// Remove the entity from the zone.
	m_GameWorld->RemoveUser(clientId);
	m_activeClients--;

	// Notify the other network clients that this user went offline.
	message.type = MSG_USER_DISCONNECT;
	message.idNumber = clientId;

	for(i=0; i<MAX_CLIENTS; i++)
	{
		if(m_clientList[i].online)
		{
			bytesSent = sendto(m_Socket, (char*)&message, sizeof(MSG_USER_DISCONNECT_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));

			if(bytesSent != sizeof(MSG_USER_DISCONNECT_DATA))
			{
				cout << "WARNING-0050: Error sending user disconnect message to client." << endl;
			}
		}
	}

	return;
}

bool NWSvr::VerifyUser(unsigned short clientId, unsigned short sessionId, int queuePosition)
{
	char *ipAddress1, *ipAddress2;


	// Check if the client should still be online.
	if(m_clientList[clientId].online == false)
	{
		cout << "WARNING: Network message from client that should be disconnected." << endl;
		return false;
	}

	// Check that the session ID is correct for this client.
	if(m_clientList[clientId].sessionID != sessionId)
	{
		cout << "WARNING : False session id for client." << endl;
		return false;
	}

	// Check the IP address that sent this message actually corresponds to the original IP address of the client.
	ipAddress1 = inet_ntoa(m_clientList[clientId].clientAddress.sin_addr);
	ipAddress2 = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);

	if(strcmp(ipAddress1, ipAddress2) != 0)
	{
		cout << "WARNING : Incorrect IP address for client." << endl;
		return false;
	}

	return true;
}

void NWSvr::HandlePingMessage( int queuePosition )
{
	MSG_PING_DATA* iMSg;
	unsigned short clientID, sessionID;
	char* ipAddress;
	MSG_GENERIC_DATA oMSG;

	iMSg = (MSG_PING_DATA*)m_networkMessageQueue[queuePosition].message;
	clientID = iMSg->idNumber;
	sessionID = iMSg->sessionId;

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	// cout << "Received a PING message from " << ipAddress << " sending PONG..." << endl;

	oMSG.type = MSG_PING;

	int bytesSent = sendto(m_Socket, (char*)&oMSG, sizeof(MSG_GENERIC_DATA), 0, (struct sockaddr*)&m_clientList[clientID].clientAddress, sizeof(m_clientList[clientID].clientAddress));
	if (bytesSent != sizeof(MSG_GENERIC_DATA))
		cout << "WARNING : Could not send PONG message to client, start countdown to drop." << endl;
}
void NWSvr::HandleChatMessage(int queuePosition)
{
	MSG_CHAT_DATA* iMsg;
	unsigned short clientID, sessionID;
	char* ipAddress;
	MSG_CHAT_DATA message;

	iMsg = (MSG_CHAT_DATA*)m_networkMessageQueue[queuePosition].message;

	clientID = iMsg->idNumber;
	sessionID = iMsg->sessionId;

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	cout << "User : " << iMsg->sessionId << " says : " << iMsg->text << " Relaying.." << endl;

	// Create the chat message to send to all (other) clients.
	message.type = MSG_CHAT;
	message.idNumber = clientID;
	strcpy(message.text, iMsg->text);

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		if (m_clientList[i].online)
		{
			if (i != clientID)
			{
				int bytesSent = sendto(m_Socket, (char*)&message, sizeof(MSG_CHAT_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
				if (bytesSent != sizeof(MSG_CHAT_DATA))
				{
					cout << "Could not relay message to client : " << m_clientList[i].sessionID << endl;
				}
			}
		}
	}
}
void NWSvr::HandleEntityRequestMessage(int queuePosition)
{
	MSG_SIMPLE_DATA* iMsg;
	unsigned short clientID, sessionID;
	char* ipAddress;
	int numEntities;
	unsigned short ID;
	char type;
	XMFLOAT3 position, rotation;
	MSG_ENTITY_INFO_DATA oMsg;

	iMsg = (MSG_SIMPLE_DATA*)m_networkMessageQueue[queuePosition].message;

	clientID = iMsg->idNumber;
	sessionID = iMsg->sessionId;

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);
	cout << "Received an entity list request from " << clientID << ", responding.." << endl;

	if (!m_GameWorld)
	{
		cout << "No active map is being played, invalid request." << endl;
		return;
	}

	numEntities = m_GameWorld->GetEntityCount();

	for (int i = 0; i < numEntities; i++)
	{
		m_GameWorld->GetEntityData(i, ID, type, position, rotation);

		if ((type == ENTITY_TYPE_USER) && (ID == clientID))
		{
			// If the requesting user is the current index, don't reply.
		}
		else
		{
			oMsg.type = MSG_ENTITY_INFO;
			oMsg.entityId = ID;
			oMsg.entityType = type;
			oMsg.position = position;
			oMsg.rotation = rotation;
			int bytesSent = sendto(m_Socket, (char*)&oMsg, sizeof(MSG_ENTITY_INFO_DATA), 0, (struct sockaddr*)&m_clientList[clientID].clientAddress, sizeof(m_clientList[clientID].clientAddress));
			if (bytesSent != sizeof(MSG_ENTITY_INFO_DATA))
				cout << "Could not sent entity info message." << endl;
		}
	}
}

void NWSvr::HandleStateChangeMessage(int queuePosition)
{

}

void NWSvr::HandlePositionMessage(int queuePosition)
{
	MSG_POSITION_DATA* iMsg;
	unsigned short clientID, sessionID;
	unsigned long stamp;
	XMFLOAT3 position, rotation, velocity, acceleration;
	char* ipAddress;
	MSG_POSITION_DATA oMsg;

	bool relay = false;

	iMsg = (MSG_POSITION_DATA*)m_networkMessageQueue[queuePosition].message;

	clientID = iMsg->idNumber;
	sessionID = iMsg->sessionId;
	stamp = iMsg->timeStamp;

	position = iMsg->position;
	rotation = iMsg->rotation;
	velocity = iMsg->velocity;
	acceleration = iMsg->acceleration;

	ipAddress = inet_ntoa(m_networkMessageQueue[queuePosition].address.sin_addr);

	relay = m_GameWorld->SetData(clientID, position, rotation, stamp);

	if (relay)
	{
		// Interpolation position and predict
	
		oMsg.type = MSG_POSITION;
		oMsg.idNumber = clientID;
		oMsg.position = position;
		oMsg.rotation = rotation;
		oMsg.velocity = velocity;
		oMsg.acceleration = acceleration;

		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (m_clientList[i].online)
			{
				if (i != clientID)
				{
					int bytesSent = sendto(m_Socket, (char*)&oMsg, sizeof(MSG_POSITION_DATA), 0, (struct sockaddr*)&m_clientList[i].clientAddress, sizeof(m_clientList[i].clientAddress));
					if (bytesSent != sizeof(MSG_POSITION_DATA))
					{
						cout << "Sending of position message failed on client : " << clientID << endl;
					}
				}

			}
		}
	}
	else
	{
		cout << "Packet sent out of order, ignoring...\n";
	}
}

void ServerListenFunc(void* ptr)
{
	cout << "Server is now listening for messages" << std::endl;

	NWSvr* svrPtr = 0;
	int clientLength;
	int bytesRead = 0;
	char recvBuffer[4096];
	struct sockaddr_in clientAddress;
	unsigned long time = timeGetTime();
	double tick = svrPtr->GetTickRate() / 60.0f;
	//timeGetTime() >= (time + tick))

	svrPtr = (NWSvr*)ptr;
	clientLength = sizeof(clientAddress);

	if (svrPtr != NULL)
	{
		while (svrPtr->Online())
		{
			if (timeGetTime() >= (time + tick))
			{
				cout << "Tick...\n";
				svrPtr->Pete();
				bytesRead = recvfrom(svrPtr->GetServerSocket(), recvBuffer, 4096, 0, (struct sockaddr*)&clientAddress, &clientLength);

				if (bytesRead > 0)
				{
					cout << "Message received" << endl;
					svrPtr->AddMSGToQ(recvBuffer, bytesRead, clientAddress);
				}

				time = timeGetTime();

			}
		}
	}
}