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
	PKT_C_CHAT = 1000,
	PKT_S_CHAT = 1001,
};

bool	Handle_INVALID(function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len);
void	Handle_S_CHAT(const PacketSessionRef& session, const Protocol::S_CHAT& pkt);

class ServerPacketHandler
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_CHAT] = [](function<void()>& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::S_CHAT>(outFunc, Handle_S_CHAT, session, buffer, len);
		};
	}

	static bool	PacketHandler(function<void()>& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT); }

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
		pkt.SerializeToArray(sendBuffer->GetCopyBuffer(), pktSize);
		sendBuffer->OnWrite(pktSize);
		return sendBuffer;
	}
};