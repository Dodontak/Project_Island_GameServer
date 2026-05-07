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
{%- for pkt in parser.total_pkt %}
	PKT_{{pkt.name}} = {{pkt.id}},
{%- endfor %}
};

bool	Handle_INVALID(DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len);
{%- for pkt in parser.recv_pkt %}
void	Handle_{{pkt.name}}(const PacketSessionRef& session, const Protocol::{{pkt.name}}& pkt);
{%- endfor %}

class {{ output }}
{
public:
	static void	Init()
	{
		for (int i = 0; i < UINT16_MAX; ++i)
			GPacketHandler[i] = Handle_INVALID;
{%- for pkt in parser.recv_pkt %}
		GPacketHandler[PKT_{{pkt.name}}] = [](DeferredFunc& outFunc, PacketSessionRef& session, BYTE* buffer, int32 len) {
			return GetCallback<Protocol::{{pkt.name}}>(outFunc, Handle_{{pkt.name}}, session, buffer, len);
		};
{%- endfor %}
	}

	static bool	PacketHandler(DeferredFunc& outFunc, PacketSessionRef session, BYTE* buffer, int32 len)
	{
		PacketHeader*	header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](outFunc, session, buffer, len);
	}

{%- for pkt in parser.send_pkt %}
	static SendBufferRef MakeSendBuffer(Protocol::{{pkt.name}}& pkt) { return MakeSendBuffer(pkt, PKT_{{pkt.name}}); }
{%- endfor %}

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