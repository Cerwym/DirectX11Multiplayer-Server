#ifndef GAMEWORLD_H
#define GAMEWORLD_H
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <DirectXMath.h>
#include "NWEntity.h"

using namespace std;
using namespace DirectX;

const int MAX_ENTITES = 20;

class GameWorld
{
public:
	GameWorld();
	GameWorld(const GameWorld& other){}
	~GameWorld();

	bool Init();
	void Update();

	void AddNewUser(unsigned short id, SOCKET sock = NULL);
	int GetEntityCount(){return m_EntityCount;}
	void GetEntityData(int, unsigned short&, char&, XMFLOAT3& position, XMFLOAT3& rotation);
	void RemoveUser(unsigned short);
	void SetStateChanged(unsigned short, char);
	bool SetData(unsigned short, XMFLOAT3 position, XMFLOAT3 rotation, unsigned long timestamp);
	void GetNetworkMessage(int&, unsigned short&);

private:
	void NetworkMessage(int, unsigned short);

	NWEntity* m_EntityList;
	int m_EntityCount;
	int m_networkMessage;
	unsigned short m_networkID;
};

#endif