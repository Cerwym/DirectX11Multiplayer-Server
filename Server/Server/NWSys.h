#ifndef NWSYS_H
#define NWSYS_H

#include "NWSvr.h"
#include "GameWorld.h"

class NWSys
{
public:

	NWSys();
	NWSys(const NWSys& other){}
	~NWSys();

	bool Init(float tickRate);
	void Update();

	bool Online(){ return m_IsOnline;}

private:
	bool m_IsOnline;
	NWSvr* m_Network;
	GameWorld* m_World;
};

#endif