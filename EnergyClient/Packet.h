#ifndef PACKET_H
#define PACKET_H

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

// 协议常量定义
constexpr uint16_t PROTOCOL_HEAD = 0xCFCF; // [cite: 9]
constexpr uint8_t TYPE_AUTH = 0x03;       // [cite: 11]
constexpr uint8_t TYPE_HEARTBEAT = 0x05;  // [cite: 12]
constexpr uint8_t TYPE_ENERGY = 0x07;     // [cite: 14]

#pragma pack(push, 1)
struct ProtocolHeader {
	uint16_t head;   // 2字节，0xCFCF [cite: 9]
	uint8_t  type;   // 1字节，消息类型 [cite: 10]
	uint32_t length; // 4字节，网络字节序，Data长度 
};
#pragma pack(pop)

struct Packet {
	ProtocolHeader header;
	std::string data; // XML 明文数据 [cite: 16]

	// 序列化为网络字节流
	std::vector<uint8_t> serialize() const {
		std::vector<uint8_t> buffer(sizeof(ProtocolHeader) + data.size());

		ProtocolHeader netHeader;
		netHeader.head = htons(header.head);
		netHeader.type = header.type;
		netHeader.length = htonl(static_cast<uint32_t>(data.size())); // 

		std::memcpy(buffer.data(), &netHeader, sizeof(ProtocolHeader));//内存拷贝 std::memcpy(target_addr,src_addr,sizeof(src_addr))
		if (!data.empty()) {
			std::memcpy(buffer.data() + sizeof(ProtocolHeader), data.data(), data.size());
		}
		return buffer;
	}
};

#endif // PACKET_H
