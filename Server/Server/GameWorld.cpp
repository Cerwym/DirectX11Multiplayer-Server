#include "GameWorld.h"

GameWorld::GameWorld()
{
	m_EntityList = 0;
}

GameWorld::~GameWorld()
{
	if (m_EntityList)
	{
		delete [] m_EntityList;
		m_EntityList = 0;
	}
}

bool GameWorld::Init()
{
	m_EntityList = new NWEntity[MAX_ENTITES];
	if (!m_EntityList)
		return false;

	// Could possibly add some entities here that exist only on the server
	// i.e AI bots

	m_EntityCount = 0;
	m_networkMessage = 0;

	return true;
}

void GameWorld::Update()
{
	// process AI information
}

void GameWorld::AddNewUser(unsigned short id, SOCKET sock)
{
	int i = 0;

	while ((m_EntityList[i].IsActive() == true) && (i < MAX_ENTITES))
	{
		i ++;
	}

	if (i != MAX_ENTITES)
	{
		m_EntityList[i].SetSocket(sock);
		m_EntityList[i].SetActive(true);
		m_EntityList[i].SetID(id);
		m_EntityList[i].SetType(ENTITY_TYPE_USER);
		// For set position, would probably want to expand this to pick a possible spawn location in the
		m_EntityList[i].SetPosition(XMFLOAT3(20.0f, 1.0f, 20.0f));
		m_EntityList[i].SetRotation(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_EntityCount++;
	}
	else
	{
		cout << "Warning : Max Players reached" << endl;
	}
}

void GameWorld::GetEntityData(int index, unsigned short& id, char& type, XMFLOAT3& position, XMFLOAT3& rotation)
{
	m_EntityList[index].GetID(id);
	m_EntityList[index].GetType(type);
	m_EntityList[index].GetPosition(position);
	m_EntityList[index].GetRotation(rotation);
}

void GameWorld::RemoveUser(unsigned short id)
{
	int i = 0;
	bool found = false;
	unsigned short entityID;

	while (!found)
	{
		m_EntityList[i].GetID(entityID);

		if (entityID == id)
			found = true;
		else
			i++;
	}

	m_EntityList[i].SetActive(false);
	m_EntityList--;
}

void GameWorld::SetStateChanged(unsigned short clientID, char state)
{

}

bool GameWorld::SetData(unsigned short clientID, XMFLOAT3 position, XMFLOAT3 rotation, unsigned long timestamp)
{
	int i = 0;
	bool found = false;
	unsigned short entityID;

	while (!found)
	{
		m_EntityList[i].GetID(entityID);

		if (entityID == clientID)
			found = true;
		else
			i++;
	}

	if (m_EntityList[i].GetTimeStamp() < timestamp)
	{
		m_EntityList[i].SetTimestamp(timestamp);
		m_EntityList[i].SetPosition(position);
		m_EntityList[i].SetRotation(rotation);

		return true;
	}

	cout << "Difference between packets : " << timestamp - m_EntityList[i].GetTimeStamp();

	return false;
}

void GameWorld::NetworkMessage(int messageType, unsigned short IDNumber)
{
	m_networkMessage = messageType;
	m_networkID = IDNumber;
}

void GameWorld::GetNetworkMessage(int & type, unsigned short& ID)
{
	type = m_networkMessage;
	ID = m_networkID;
	m_networkMessage = 0;
}