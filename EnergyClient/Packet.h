#ifndef PACKET_H
#define PACKET_H

#include <string>
#include <cstdint>


constexpr uint16_t PROTOCOL_HEAD = 0xCFCF; // 协议常量定义 [cite: 9, 10]
constexpr uint8_t TYPE_AUTH = 0x03;       // [cite: 11]
constexpr uint8_t TYPE_HEARTBEAT = 0x05;  // [cite: 12]
constexpr uint8_t TYPE_ENERGY = 0x07;     // [cite: 14]

#pragma pack(push, 1)
struct ProtocolHeader {
	uint16_t head;   // 2字节，固定为0xCFCF 
	uint8_t  type;   // 1字节，消息类型 [cite: 10]
	uint32_t length; // 4字节，Data长度（网络字节序） 
};
#pragma pack(pop)

// 纯粹的业务报文载体 
struct Packet {
	uint8_t type = 0;
	std::string data; // XML 明文数据 [cite: 16]
};

#endif // PACKET_H