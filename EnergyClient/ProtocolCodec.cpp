#include "ProtocolCodec.h"
#include <cstring>
#if defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

std::vector<uint8_t> ProtocolCodec::encode(const Packet& packet) {
	std::vector<uint8_t> bytes(sizeof(ProtocolHeader) + packet.data.size());

	ProtocolHeader header;
	header.head = htons(PROTOCOL_HEAD); // 转换为网络字节序 
	header.type = packet.type;
	header.length = htonl(static_cast<uint32_t>(packet.data.size())); // 转换为网络字节序 

	std::memcpy(bytes.data(), &header, sizeof(ProtocolHeader));
	if (!packet.data.empty()) {
		std::memcpy(bytes.data() + sizeof(ProtocolHeader), packet.data.data(), packet.data.size());
	}
	return bytes;
}

bool ProtocolCodec::decode(std::vector<uint8_t>& buffer, Packet& outPacket) {
	// 长度不足一个包头，继续等待 
	if (buffer.size() < sizeof(ProtocolHeader)) {
		return false;
	}

	// 预读取包头，不移动游标 
	ProtocolHeader header;
	std::memcpy(&header, buffer.data(), sizeof(ProtocolHeader));

	uint16_t head = ntohs(header.head);
	uint32_t length = ntohl(header.length); // 

	if (head != PROTOCOL_HEAD) {
		// 发现非法协议头，说明流已经错乱，清空丢弃防死锁（高异常处理规范） [cite: 107]
		buffer.erase(buffer.begin(), buffer.begin() + 1);
		return false;
	}

	// 判断当前缓冲区里的数据是否满足包头指明的完整大小 [cite: 8, 15]
	if (buffer.size() < sizeof(ProtocolHeader) + length) {
		return false; // 半包情况，等待下次物理接收
	}

	// 提取完整数据包 
	outPacket.type = header.type;
	outPacket.data.assign(reinterpret_cast<char*>(buffer.data() + sizeof(ProtocolHeader)), length);

	// 从缓冲区移除已消费的字节流（解决粘包核心）
	buffer.erase(buffer.begin(), buffer.begin() + sizeof(ProtocolHeader) + length);
	return true;
}