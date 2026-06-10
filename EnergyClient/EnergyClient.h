#ifndef ENERGY_CLIENT_H
#define ENERGY_CLIENT_H

#include "TcpClient.h"
#include "Packet.h"
#include <string>
#include <chrono>

class EnergyClient {
private:
	TcpClient m_netPipe; // 持有底层网络管道
	std::string m_buildingId = "XH310104BG0321";
	std::string m_gatewayId = "1";

	std::chrono::steady_clock::time_point m_lastHeartbeatTime;
	std::chrono::steady_clock::time_point m_lastDataTime;

	std::string getCurrentTimeStr();
	bool executeAuth();
	bool waitForPacket(Packet& outPack, int timeoutMs = 5000);

public:
	EnergyClient(std::string buildingId, std::string gatewayId);
	bool run(const std::string& ip, int port);
};

#endif // ENERGY_CLIENT_H