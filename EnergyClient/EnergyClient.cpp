#include "EnergyClient.h"
#include "../Protocol/ProtocolCodec.h"
#include "../XML/XmlBuilder.h"
#include "../XML/XmlParser.h"
#include "../Utils/Md5Util.h"
#include <iostream>
#include <thread>
#include <sstream>
#include <iomanip>

EnergyClient::EnergyClient(std::string buildingId, std::string gatewayId)
	: m_buildingId(std::move(buildingId)), m_gatewayId(std::move(gatewayId)) {}

std::string EnergyClient::getCurrentTimeStr() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S");
	return ss.str();
}

// 结合协议编解码器进行数据包的同步阻塞获取（含超时控制，防死锁异常） [cite: 107]
bool EnergyClient::waitForPacket(Packet& outPack, int timeoutMs) {
	auto start = std::chrono::steady_clock::now();
	while (m_netPipe.isConnected()) {
		// 1. 尝试从已有流缓冲区解包 [cite: 6]
		if (ProtocolCodec::decode(m_netPipe.getBuffer(), outPack)) {
			return true;
		}
		// 2. 缓冲区不足以解出完整包，继续从网络读取
		m_netPipe.readToBuffer();

		auto now = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() > timeoutMs) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
	}
	return false;
}

bool EnergyClient::executeAuth() {
	try {
		// (1) 发送身份认证请求 [cite: 19]
		// 业务层只负责把XML内容和塞进包裹
		Packet reqPack{ TYPE_AUTH, XmlBuilder::buildAuthRequest(m_buildingId, m_gatewayId) };
		if (!m_netPipe.sendRaw(ProtocolCodec::encode(reqPack))) return false; [cite:15]

		// (2) 接收挑战序列 [cite: 20]
		Packet resPack;
		if (!waitForPacket(resPack)) return false;
		std::string sequence = XmlParser::extractTagValue(resPack.data, "sequence"); [cite:44]

		// (3) 计算 MD5 并送回平台 [cite: 21]
		std::string md5Val = Md5Util::compute(sequence);
		Packet md5Pack{ TYPE_AUTH, XmlBuilder::buildAuthMd5(m_buildingId, m_gatewayId, md5Val) };
		if (!m_netPipe.sendRaw(ProtocolCodec::encode(md5Pack))) return false;// [cite:15]

		// (4) 判定最终接入结果 [cite: 22]
		if (!waitForPacket(resPack)) return false;
		std::string result = XmlParser::extractTagValue(resPack.data, "result"); //[cite:49]

		return (result == "pass"); // [cite: 22, 49]
	}
	catch (const std::exception& e) {
		std::cerr << "[认证异常]: " << e.what() << std::endl; //[cite:107]
		return false;
	}
}

bool EnergyClient::run(const std::string& ip, int port) {
	if (!m_netPipe.connectToServer(ip, port)) return false;

	std::cout << "[Client] 网络通道已联通，启动安全规约认证流程..." << std::endl; //[cite:19]
	if (!executeAuthWorkflow()) {
		std::cerr << "[Client] 规约安全认证未通过，断开连接." << std::endl; //[cite:22]
		m_netPipe.disconnect();
		return false;
	}
	std::cout << "[Client] 规约认证成功. 开启能耗定时调度状态机." << std::endl; //[cite:22, 24]

	m_lastHeartbeatTime = std::chrono::steady_clock::now();
	m_lastDataTime = std::chrono::steady_clock::now();

	// (5) 执行定时任务循环 [cite: 24]
	while (m_netPipe.isConnected()) {
		auto now = std::chrono::steady_clock::now();

		// 1分钟发送一次心跳包 [cite: 24]
		if (std::chrono::duration_cast<std::chrono::minutes>(now - m_lastHeartbeatTime).count() >= 1) {
			Packet hb{ TYPE_HEARTBEAT, XmlBuilder::buildHeartbeat(m_buildingId, m_gatewayId, getCurrentTimeStr()) };
			m_netPipe.sendRaw(ProtocolCodec::encode(hb)); //[cite:15]
			m_lastHeartbeatTime = now;
			std::cout << "[Job] 心跳校时报文已同步." << std::endl; //[cite:50]
		}

		// 5分钟发送一次数据包 [cite: 24]
		if (std::chrono::duration_cast<std::chrono::minutes>(now - m_lastDataTime).count() >= 5) {
			std::vector<EnergyItem> items = { {"01000", "45.2"}, {"01A00", "12.8"} }; //[cite:78, 79]
			Packet data{ TYPE_ENERGY, XmlBuilder::buildEnergyReport(m_buildingId, m_gatewayId, getCurrentTimeStr(), items) };
			m_netPipe.sendRaw(ProtocolCodec::encode(data)); //[cite:15]
			m_lastDataTime = now;
			std::cout << "[Job] 周期性物理能耗数据上报成功." << std::endl; //[cite:67]
		}

		// 维持非阻塞轮询读取，保持 TCP 缓冲区被及时消费，防止对端发回应答时造成 TCP 阻塞窗口死锁
		m_netPipe.readToBuffer();

		// 尝试消费查看是否有平台回复的应答，保持长连接健康
		Packet dummy;
		while (ProtocolCodec::decode(m_netPipe.getBuffer(), dummy)) {
			// 可在此扩展对应答包（ACK）或校时返回的处理逻辑 [cite: 64, 91]
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
	return true;
}