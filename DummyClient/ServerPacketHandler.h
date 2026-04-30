#pragma once

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
#include "Client.h"
#else
#include "Types.h"
#include "Session.h"
#include "SendBuffer.h"
#include <memory>
#include <functional>
#endif
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
	PKT_C_CHAT = 1004,
	PKT_S_CHAT = 1005,
};

bool	Handle_INVALID(DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len);
void	Handle_S_LOGIN(const PacketSessionRef& session, const Protocol::S_LOGIN& pkt);
void	Handle_S_ENTER_ROOM(const PacketSessionRef& session, const Protocol::S_ENTER_ROOM& pkt);
void	Handle_S_CHAT(const PacketSessionRef& session, const Protocol::S_CHAT& pkt);

class ServerPacketHandler
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::S_LOGIN>(outFunc, Handle_S_LOGIN, session, buffer, len);
		};
		GPacketHandler[PKT_S_ENTER_ROOM] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::S_ENTER_ROOM>(outFunc, Handle_S_ENTER_ROOM, session, buffer, len);
		};
		GPacketHandler[PKT_S_CHAT] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::S_CHAT>(outFunc, Handle_S_CHAT, session, buffer, len);
		};
	}

	static bool	PacketHandler(DeferredFunc& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ENTER_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_ENTER_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT); }

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