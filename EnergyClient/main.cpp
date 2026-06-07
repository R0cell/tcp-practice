#include "EnergyClient/EnergyClient.h"
#include <iostream>

int main() {
	// 实例化客户端（传入大楼ID与网关ID）
	EnergyClient client("XH310104BG0321", "1"); // [cite: 42]

	// 假设上层平台服务的IP及端口为 127.0.0.1 : 8008
	std::string platformIp = "127.0.0.1";
	int platformPort = 8008;

	try {
		client.run(platformIp, platformPort);
	}
	catch (const std::exception& e) {
		std::cerr << "致命全局异常: " << e.what() << std::endl; // [cite: 107]
	}

	return 0;
}