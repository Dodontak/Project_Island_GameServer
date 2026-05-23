#pragma once

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
#include "Client.h"
#else
#include "Types.h"
#include "Session.h"
#include <memory>
#include <functional>
#endif
#include "SendBuffer.h"
#include "Protocol.pb.h"

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
using DeferredFunc = TFunction<void()>;
using PacketHandlerFunc = TFunction<bool(DeferredFunc&, PacketSessionRef&, BYTE*, int32)>;
#else
using DeferredFunc = std::function<void()>;
using PacketHandlerFunc = std::function<bool(DeferredFunc&, PacketSessionRef&, BYTE*, int32)>;
#endif

extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_GC_LOGIN = 10000,
	PKT_GS_LOGIN = 10001,
	PKT_GC_CHARACTER_LIST = 10002,
	PKT_GS_CHARACTER_LIST = 10003,
	PKT_GC_CHECK_NICKNAME = 10004,
	PKT_GS_CHECK_NICKNAME = 10005,
	PKT_GC_CREATE_CHARACTER = 10006,
	PKT_GS_CREATE_CHARACTER = 10007,
	PKT_GC_ENTER_ROOM = 10008,
	PKT_GS_ENTER_ROOM = 10009,
	PKT_GC_LEAVE_ROOM = 10010,
	PKT_GS_LEAVE_ROOM = 10011,
	PKT_GC_LEAVE_GAME = 10012,
	PKT_GS_LEAVE_GAME = 10013,
	PKT_GS_SPAWN = 10014,
	PKT_GS_DESPAWN = 10015,
	PKT_GC_CHAT = 10016,
	PKT_GS_CHAT = 10017,
	PKT_AC_SIGNUP = 20000,
	PKT_AS_SIGNUP = 20001,
	PKT_AC_VERIFY_MAIL_REQ = 20002,
	PKT_AS_VERIFY_MAIL_REQ = 20003,
	PKT_AC_VERIFY_EMAIL_CODE = 20004,
	PKT_AS_VERIFY_EMAIL_CODE = 20005,
	PKT_AC_LOGIN = 20006,
	PKT_AS_LOGIN = 20007,
};

bool	Handle_INVALID(DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len);
void	Handle_GS_LOGIN(const PacketSessionRef& session, const Protocol::GS_LOGIN& pkt);
void	Handle_GS_CHARACTER_LIST(const PacketSessionRef& session, const Protocol::GS_CHARACTER_LIST& pkt);
void	Handle_GS_CHECK_NICKNAME(const PacketSessionRef& session, const Protocol::GS_CHECK_NICKNAME& pkt);
void	Handle_GS_CREATE_CHARACTER(const PacketSessionRef& session, const Protocol::GS_CREATE_CHARACTER& pkt);
void	Handle_GS_ENTER_ROOM(const PacketSessionRef& session, const Protocol::GS_ENTER_ROOM& pkt);
void	Handle_GS_LEAVE_ROOM(const PacketSessionRef& session, const Protocol::GS_LEAVE_ROOM& pkt);
void	Handle_GS_LEAVE_GAME(const PacketSessionRef& session, const Protocol::GS_LEAVE_GAME& pkt);
void	Handle_GS_SPAWN(const PacketSessionRef& session, const Protocol::GS_SPAWN& pkt);
void	Handle_GS_DESPAWN(const PacketSessionRef& session, const Protocol::GS_DESPAWN& pkt);
void	Handle_GS_CHAT(const PacketSessionRef& session, const Protocol::GS_CHAT& pkt);
void	Handle_AS_SIGNUP(const PacketSessionRef& session, const Protocol::AS_SIGNUP& pkt);
void	Handle_AS_VERIFY_MAIL_REQ(const PacketSessionRef& session, const Protocol::AS_VERIFY_MAIL_REQ& pkt);
void	Handle_AS_VERIFY_EMAIL_CODE(const PacketSessionRef& session, const Protocol::AS_VERIFY_EMAIL_CODE& pkt);
void	Handle_AS_LOGIN(const PacketSessionRef& session, const Protocol::AS_LOGIN& pkt);

class ServerPacketHandler
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_GS_LOGIN] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_LOGIN>(outFunc, Handle_GS_LOGIN, session, buffer, len);
		};
		GPacketHandler[PKT_GS_CHARACTER_LIST] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_CHARACTER_LIST>(outFunc, Handle_GS_CHARACTER_LIST, session, buffer, len);
		};
		GPacketHandler[PKT_GS_CHECK_NICKNAME] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_CHECK_NICKNAME>(outFunc, Handle_GS_CHECK_NICKNAME, session, buffer, len);
		};
		GPacketHandler[PKT_GS_CREATE_CHARACTER] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_CREATE_CHARACTER>(outFunc, Handle_GS_CREATE_CHARACTER, session, buffer, len);
		};
		GPacketHandler[PKT_GS_ENTER_ROOM] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_ENTER_ROOM>(outFunc, Handle_GS_ENTER_ROOM, session, buffer, len);
		};
		GPacketHandler[PKT_GS_LEAVE_ROOM] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_LEAVE_ROOM>(outFunc, Handle_GS_LEAVE_ROOM, session, buffer, len);
		};
		GPacketHandler[PKT_GS_LEAVE_GAME] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_LEAVE_GAME>(outFunc, Handle_GS_LEAVE_GAME, session, buffer, len);
		};
		GPacketHandler[PKT_GS_SPAWN] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_SPAWN>(outFunc, Handle_GS_SPAWN, session, buffer, len);
		};
		GPacketHandler[PKT_GS_DESPAWN] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_DESPAWN>(outFunc, Handle_GS_DESPAWN, session, buffer, len);
		};
		GPacketHandler[PKT_GS_CHAT] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::GS_CHAT>(outFunc, Handle_GS_CHAT, session, buffer, len);
		};
		GPacketHandler[PKT_AS_SIGNUP] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_SIGNUP>(outFunc, Handle_AS_SIGNUP, session, buffer, len);
		};
		GPacketHandler[PKT_AS_VERIFY_MAIL_REQ] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_VERIFY_MAIL_REQ>(outFunc, Handle_AS_VERIFY_MAIL_REQ, session, buffer, len);
		};
		GPacketHandler[PKT_AS_VERIFY_EMAIL_CODE] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_VERIFY_EMAIL_CODE>(outFunc, Handle_AS_VERIFY_EMAIL_CODE, session, buffer, len);
		};
		GPacketHandler[PKT_AS_LOGIN] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::AS_LOGIN>(outFunc, Handle_AS_LOGIN, session, buffer, len);
		};
	}

	static bool	PacketHandler(DeferredFunc& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::GC_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_GC_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::GC_CHARACTER_LIST& pkt) { return MakeSendBuffer(pkt, PKT_GC_CHARACTER_LIST); }
	static SendBufferRef MakeSendBuffer(Protocol::GC_CHECK_NICKNAME& pkt) { return MakeSendBuffer(pkt, PKT_GC_CHECK_NICKNAME); }
	static SendBufferRef MakeSendBuffer(Protocol::GC_CREATE_CHARACTER& pkt) { return MakeSendBuffer(pkt, PKT_GC_CREATE_CHARACTER); }
	static SendBufferRef MakeSendBuffer(Protocol::GC_ENTER_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_GC_ENTER_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::GC_LEAVE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_GC_LEAVE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::GC_LEAVE_GAME& pkt) { return MakeSendBuffer(pkt, PKT_GC_LEAVE_GAME); }
	static SendBufferRef MakeSendBuffer(Protocol::GC_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_GC_CHAT); }
	static SendBufferRef MakeSendBuffer(Protocol::AC_SIGNUP& pkt) { return MakeSendBuffer(pkt, PKT_AC_SIGNUP); }
	static SendBufferRef MakeSendBuffer(Protocol::AC_VERIFY_MAIL_REQ& pkt) { return MakeSendBuffer(pkt, PKT_AC_VERIFY_MAIL_REQ); }
	static SendBufferRef MakeSendBuffer(Protocol::AC_VERIFY_EMAIL_CODE& pkt) { return MakeSendBuffer(pkt, PKT_AC_VERIFY_EMAIL_CODE); }
	static SendBufferRef MakeSendBuffer(Protocol::AC_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_AC_LOGIN); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool	GetCallback(DeferredFunc& outFunc, ProcessFunc func, PacketSessionRef session, BYTE* buffer, int32 len)
	{
		PacketType	pkt;
		if (false == pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)))
			return false;
		outFunc = [func, session, pkt](){ func(session, pkt); };
		return true;
	}

	template<typename T>
	static SendBufferRef	MakeSendBuffer(T& pkt, uint16 pktId)
	{
		int	headerSize = sizeof(PacketHeader);
		int	pktSize = pkt.ByteSizeLong();
		#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
				SendBufferRef sendBuffer = MakeShared<SendBuffer>(headerSize + pktSize);
		#else
				SendBufferRef sendBuffer = make_shared<SendBuffer>(headerSize + pktSize);
		#endif
		PacketHeader	header;
		header.id = pktId;
		header.size = headerSize + pktSize;

		sendBuffer->AppendBuffer(reinterpret_cast<BYTE*>(&header), headerSize);
		if (pktSize > 0)
		{
			pkt.SerializeToArray(sendBuffer->WritePos(), pktSize);
			sendBuffer->OnWrite(pktSize);
		}
		return sendBuffer;
	}
};