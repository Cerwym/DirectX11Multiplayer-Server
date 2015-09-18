#ifndef NWENTITY_H
#define NWENTITY_H

#include <WinSock2.h>
#include <DirectXMath.h>

const char ENTITY_TYPE_USER = 0;
const char ENTITY_TYPE_AI = 1;

using namespace DirectX;

class NWEntity
{
public:

	NWEntity();
	NWEntity(const NWEntity&){};
	~NWEntity(){};

	void SetActive(bool flag)
	{
		m_active = flag;
		m_timestamp = 0;
	}
	void SetID(unsigned short id){m_id = id;}
	void SetSocket(SOCKET s){m_ClientSocket = s;}
	void SetType(char type){m_entityType = type;}
	void SetPosition(XMFLOAT3 position);
	void SetRotation(XMFLOAT3 rotation);
	void SetAcceleration(XMFLOAT3 acceleration);
	void SetTimestamp(unsigned long t){m_timestamp = t;}

	SOCKET clSocket() { return m_ClientSocket;}

	bool IsActive(){return m_active;}
	void GetID(unsigned short&);
	void GetType(char&);
	void GetPosition(XMFLOAT3 &position);
	void GetRotation(XMFLOAT3 &rotation);
	void GetAcceleration(XMFLOAT3 &acceleration);
	unsigned long GetTimeStamp(){return m_timestamp;}

private:
	SOCKET m_ClientSocket;
	bool m_active;
	unsigned short m_id;
	char m_entityType;

	unsigned long m_timestamp;
	
	XMFLOAT3 m_Position, m_Rotation, m_Acceleration;
};


#endif