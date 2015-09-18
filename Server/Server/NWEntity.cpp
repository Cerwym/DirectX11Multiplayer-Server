#include "NWEntity.h"

NWEntity::NWEntity()
{
	m_active = false;
	m_timestamp = 0;
}

void NWEntity::SetPosition(XMFLOAT3 position)
{
	m_Position = position;
}

void NWEntity::SetRotation(XMFLOAT3 rotation)
{
	m_Rotation = rotation;
}

void NWEntity::GetID(unsigned short& id)
{
	id = m_id;
}

void NWEntity::GetRotation(XMFLOAT3& rotation)
{
	rotation = m_Rotation;
}


void NWEntity::GetType(char& type)
{
	type = m_entityType;
	return;
}


void NWEntity::GetPosition(XMFLOAT3& position)
{
	position = m_Rotation;
	return;
}
