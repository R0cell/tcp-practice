#ifndef PROTOCOL_CODEC_H
#define PROTOCOL_CODEC_H

#include "Packet.h"
#include <vector>

class ProtocolCodec {
public:
	// 将 Packet 编码为可以直接用于网络发送的字节流（处理网络字节序） 
	static std::vector<uint8_t> encode(const Packet& packet);

	// 从原始缓存区尝试解析出一个完整的 Packet（处理粘包半包核心逻辑）
	// 如果解析成功返回 true，并从 buffer 中移除相应数据；数据不足返回 false
	static bool decode(std::vector<uint8_t>& buffer, Packet& outPacket);
};

#endif // PROTOCOL_CODEC_H