#include "EnergyClient.h"
#include "../XML/XmlBuilder.h"
#include "../XML/XmlParser.h"
#include "../Utils/Md5Util.h"
#include <iostream>
#include <thread>
#include <iomanip>
#include <sstream>

EnergyClient::EnergyClient(std::string buildingId, std::string gatewayId)
	: m_buildingId(std::move(buildingId)), m_gatewayId(std::move(gatewayId)) {}

std::string EnergyClient::getCurrentTimeStr() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y%m%d%H%M%S"); // format: YYYYMMDDHHMMSS [cite: 59]
	return ss.str();
}

bool EnergyClient::executeAuth() {
	try {
		// (1) 发送身份认证请求 [cite: 19]
		Packet reqPacket{ {PROTOCOL_HEAD, TYPE_AUTH, 0}, XmlBuilder::buildAuthRequest(m_buildingId, m_gatewayId) }; // [cite: 9, 11, 42]
		if (!m_client.sendPacket(reqPacket)) return false;
		std::cout << "[Auth] 认证第一步：请求发送完毕." << std::endl;

		// (2) 接收上层平台发送的随机序列 [cite: 20]
		Packet resPacket;
		if (!m_client.receivePacket(resPacket)) return false;
		std::string sequence = XmlParser::extractTagValue(resPacket.data, "sequence"); // [cite: 44]
		std::cout << "[Auth] 认证第二步：获取挑战序列 -> " << sequence << std::endl;

		// (3) 计算 MD5 并发送给平台 [cite: 21]
		std::string md5Result = Md5Util::compute(sequence);
		Packet md5Packet{ {PROTOCOL_HEAD, TYPE_AUTH, 0}, XmlBuilder::buildAuthMd5(m_buildingId, m_gatewayId, md5Result) }; // [cite: 9, 11, 46]
		if (!m_client.sendPacket(md5Packet)) return false;

		// (4) 接收最终结果 [cite: 22]
		if (!m_client.receivePacket(resPacket)) return false;
		std::string result = XmlParser::extractTagValue(resPacket.data, "result"); // [cite: 49]

		if (result == "pass") { // [cite: 49]
			std::cout << "[Auth] 认证第三步：校验成功！允许接入数据." << std::endl;
			return true;
		}
		else {
			std::cerr << "[Auth] 认证失败：服务器拒绝连接." << std::endl;
			return false;
		}
	}
	catch (const std::exception& e) {
		std::cerr << "[异常捕捉] 认证环节出错: " << e.what() << std::endl; // [cite: 107]
		return false;
	}
}

bool EnergyClient::run(const std::string& ip, int port) {
	std::cout << "[System] 正在连接上层能耗平台系统..." << std::endl;
	if (!m_client.connectToServer(ip, port)) {
		std::cerr << "[Error] TCP 联通失败." << std::endl;
		return false;
	}

	// 运行身份验证
	if (!executeAuth()) {
		m_client.disconnect();
		return false;
	}

	// 初始化计时
	m_lastHeartbeatTime = std::chrono::steady_clock::now();
	m_lastDataTime = std::chrono::steady_clock::now();

	// (5) 进入数据接入循环机制 
	while (m_client.isConnected()) {
		auto now = std::chrono::steady_clock::now();

		// 1 分钟发送一次心跳包 
		if (std::chrono::duration_cast<std::chrono::minutes>(now - m_lastHeartbeatTime).count() >= 1) {
			Packet hbPacket{ {PROTOCOL_HEAD, TYPE_HEARTBEAT, 0}, XmlBuilder::buildHeartbeat(m_buildingId, m_gatewayId, getCurrentTimeStr()) }; // [cite: 9, 12, 63]
			if (m_client.sendPacket(hbPacket)) {
				std::cout << "[Heartbeat] 心跳包发送成功." << std::endl;
				// 接收响应检查
				Packet ack;
				if (m_client.receivePacket(ack)) {
					std::cout << "[Heartbeat] 平台校时响应正常." << std::endl;
				}
			}
			m_lastHeartbeatTime = now;
		}

		// 5 分钟发送一次数据包 
		if (std::chrono::duration_cast<std::chrono::minutes>(now - m_lastDataTime).count() >= 5) {
			std::vector<EnergyItem> mockData = { {"01000", "125.6"}, {"01A00", "88.4"} }; // [cite: 78, 79]
			Packet dataPacket{ {PROTOCOL_HEAD, TYPE_ENERGY, 0}, XmlBuilder::buildEnergyReport(m_buildingId, m_gatewayId, getCurrentTimeStr(), mockData) }; // [cite: 9, 14, 68]

			if (m_client.sendPacket(dataPacket)) {
				std::cout << "[Data] 能耗数据包投递成功." << std::endl;
				Packet ack;
				if (m_client.receivePacket(ack)) {
					try {
						std::string status = XmlParser::extractTagValue(ack.data, "ack"); // [cite: 103]
						std::cout << "[Data] 接收到平台应答: " << status << std::endl;
					}
					catch (...) {
						std::cerr << "[Warning] 解析能耗应答出错." << std::endl; // [cite: 107]
					}
				}
			}
			m_lastDataTime = now;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 避免CPU空转
	}

	return true;
}