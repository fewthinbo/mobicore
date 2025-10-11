#include <cstdint>

#if !__MOBICORE__
typedef struct packet_messenger_list_offline
{
	BYTE connected; // always 0
	BYTE length;
} TPacketGCMessengerListOffline;

typedef struct packet_messenger_list_online
{
	BYTE connected; // always 1
	BYTE length;
} TPacketGCMessengerListOnline;
#else
typedef struct packet_messenger_user_status{
	uint16_t status{};
	uint8_t name_len{};
} TPacketGCMessengerUserStatus;
#endif

typedef struct command_chat
{
	...
	BYTE	type;
#if __MOBICORE__
	uint16_t code_page{};
#endif
} TPacketCGChat;

typedef struct command_whisper
{
	...
	char 	szNameTo[CHARACTER_NAME_MAX_LEN + 1];
#if __MOBICORE__
	uint16_t code_page{};
#endif
} TPacketCGWhisper;

enum 
{
	MESSENGER_SUBHEADER_GC_LIST,
#if !__MOBICORE__
	MESSENGER_SUBHEADER_GC_LOGIN,
	MESSENGER_SUBHEADER_GC_LOGOUT,
#else
	MESSENGER_SUBHEADER_GC_UPDATE_STATUS,
#endif
	...
}