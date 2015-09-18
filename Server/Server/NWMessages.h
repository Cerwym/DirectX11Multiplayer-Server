#ifndef NWMESSAGES_H
#define NWMESSAGES_H

#include <DirectXMath.h>

#define MSG_CONNECT 1000
#define MSG_NEWID 1001
#define MSG_PING 1002
#define MSG_DISCONNECT 1003
#define MSG_DISCONNECT 1003
#define MSG_CHAT 1004
#define MSG_ENTITY_REQUEST 1005
#define MSG_ENTITY_INFO 1006
#define MSG_NEW_USER_LOGIN 1007
#define MSG_USER_DISCONNECT 1008
#define MSG_STATE_CHANGE 1009
#define MSG_POSITION 1010

using namespace DirectX;

// unsigned short for type must come first as when the data is coerced into the generic format, type will be what identifies the message to be processed.
typedef struct           
{                        
	unsigned short type;              
}MSG_GENERIC_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
}MSG_SIMPLE_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
	XMFLOAT3 position;
}MSG_NEWID_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
}MSG_PING_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
}MSG_DISCONNECT_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
	char text[64];
}MSG_CHAT_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short entityId;
	char entityType;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
}MSG_ENTITY_INFO_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short idNumber;
}MSG_USER_DISCONNECT_DATA;

typedef struct           
{                        
	unsigned short type;
	unsigned short idNumber;
	unsigned short sessionId;
	char state;
}MSG_STATE_CHANGE_DATA;

typedef struct           
{       
	unsigned short type;
	unsigned long timeStamp;
	unsigned short idNumber;
	unsigned short sessionId;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	XMFLOAT3 velocity;
	XMFLOAT3 acceleration;
}MSG_POSITION_DATA;

#endif