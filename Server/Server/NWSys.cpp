#include "NWSys.h"

NWSys::NWSys()
{
	m_IsOnline = false;
	m_Network = 0;
	m_World = 0;
}

NWSys::~NWSys()
{
	if (m_World)
	{
		delete m_World;
		m_World = 0;
	}

	if (m_Network)
	{
		delete m_Network;
		m_Network = 0;
	}
}

bool NWSys::Init(float tickRate)
{
	m_World = new GameWorld;
	if (!m_World)
		return false;

	if (!m_World->Init())
	{
		cout << "Could not init the default game world" << endl;
		return false;
	}

	m_Network = new NWSvr;
	if (!m_Network)
		return false;

	m_Network->SetWorldInfo(m_World);

	if (!m_Network->Init(tickRate))
	{
		cout << "Could not init the network object" << endl;
		return false;
	}

	m_IsOnline = true;
	return true;
}

void NWSys::Update()
{
	int messageType;
	unsigned short id;

	m_Network->Update();

	m_World->Update();
	m_World->GetNetworkMessage(messageType, id);

	return;
}