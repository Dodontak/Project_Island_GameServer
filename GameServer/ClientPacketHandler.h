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
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_ENTER_ROOM = 1002,
	PKT_S_ENTER_ROOM = 1003,
	PKT_C_LEAVE_ROOM = 1004,
	PKT_S_LEAVE_ROOM = 1005,
	PKT_S_SPAWN = 1006,
	PKT_S_DESPAWN = 1007,
	PKT_C_CHAT = 1008,
	PKT_S_CHAT = 1009,
};

bool	Handle_INVALID(DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len);
void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt);
void	Handle_C_ENTER_ROOM(const PacketSessionRef& session, const Protocol::C_ENTER_ROOM& pkt);
void	Handle_C_LEAVE_ROOM(const PacketSessionRef& session, const Protocol::C_LEAVE_ROOM& pkt);
void	Handle_C_CHAT(const PacketSessionRef& session, const Protocol::C_CHAT& pkt);

class ClientPacketHandler
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_LOGIN] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_LOGIN>(outFunc, Handle_C_LOGIN, session, buffer, len);
		};
		GPacketHandler[PKT_C_ENTER_ROOM] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_ENTER_ROOM>(outFunc, Handle_C_ENTER_ROOM, session, buffer, len);
		};
		GPacketHandler[PKT_C_LEAVE_ROOM] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_LEAVE_ROOM>(outFunc, Handle_C_LEAVE_ROOM, session, buffer, len);
		};
		GPacketHandler[PKT_C_CHAT] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_CHAT>(outFunc, Handle_C_CHAT, session, buffer, len);
		};
	}

	static bool	PacketHandler(DeferredFunc& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ENTER_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_ENTER_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LEAVE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_LEAVE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_SPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_SPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_DESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_DESPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); }

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