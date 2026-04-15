#pragma once

#include "Types.h"
#include "Session.h"
#include "Protocol.pb.h"
#include "SendBuffer.h"
#include <memory>
#include <functional>

extern function<bool(function<void()>&, PacketSessionRef&, BYTE*, int32)> GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_ENTER_ROOM = 1002,
	PKT_S_ENTER_ROOM = 1003,
	PKT_C_CHAT = 1004,
	PKT_S_CHAT = 1005,
};

bool	Handle_INVALID(function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len);
void	Handle_C_LOGIN(const PacketSessionRef& session, const Protocol::C_LOGIN& pkt);
void	Handle_C_ENTER_ROOM(const PacketSessionRef& session, const Protocol::C_ENTER_ROOM& pkt);
void	Handle_C_CHAT(const PacketSessionRef& session, const Protocol::C_CHAT& pkt);

class ClientPacketHandler
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_LOGIN] = [](function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_LOGIN>(outFunc, Handle_C_LOGIN, session, buffer, len);
		};
		GPacketHandler[PKT_C_ENTER_ROOM] = [](function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_ENTER_ROOM>(outFunc, Handle_C_ENTER_ROOM, session, buffer, len);
		};
		GPacketHandler[PKT_C_CHAT] = [](function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::C_CHAT>(outFunc, Handle_C_CHAT, session, buffer, len);
		};
	}

	static bool	PacketHandler(function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ENTER_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_ENTER_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool	GetCallback(function<void()>& outFunc, ProcessFunc func, PacketSessionRef session, BYTE* buffer, int32 len)
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
		SendBufferRef	sendBuffer = make_shared<SendBuffer>(headerSize + pktSize);

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