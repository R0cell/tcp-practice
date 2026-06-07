#include "TcpClient.h"
#include <iostream>

#if defined(_WIN32)
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

TcpClient::TcpClient() : m_socket(-1), m_connected(false) {
#if defined(_WIN32)
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

TcpClient::~TcpClient() {
	disconnect();
#if defined(_WIN32)
	WSACleanup();
#endif
}

bool TcpClient::connectToServer(const std::string& ip, int port) {
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0) return false;

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

	if (connect(m_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
		disconnect();
		return false;
	}

	m_connected = true;
	return true;
}

void TcpClient::disconnect() {
	if (m_socket != -1) {
#if defined(_WIN32)
		closesocket(m_socket);
#else
		close(m_socket);
#endif
		m_socket = -1;
	}
	m_connected = false;
}

bool TcpClient::sendPacket(const Packet& packet) {
	if (!m_connected) return false;
	auto buffer = packet.serialize();
	int bytesSent = send(m_socket, reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0);
	return bytesSent == static_cast<int>(buffer.size());
}

bool TcpClient::receivePacket(Packet& outPacket) {
	if (!m_connected) return false;

	// 1. 准确接收 Fixed Header
	ProtocolHeader header;
	int bytesRead = recv(m_socket, reinterpret_cast<char*>(&header), sizeof(ProtocolHeader), 0);
	if (bytesRead <= 0) return false;

	// 解析 Header 并转换字节序
	outPacket.header.head = ntohs(header.head);
	outPacket.header.type = header.type;
	outPacket.header.length = ntohl(header.length); // 

	if (outPacket.header.head != PROTOCOL_HEAD) {
		std::cerr << "协议头非法校验失败." << std::endl;
		return false;
	}

	// 2. 根据明确的变长长度接收应用层 XML [cite: 15, 16]
	if (outPacket.header.length > 0) {
		std::vector<char> dataBuffer(outPacket.header.length);
		uint32_t totalReceived = 0;
		while (totalReceived < outPacket.header.length) {
			int n = recv(m_socket, dataBuffer.data() + totalReceived, outPacket.header.length - totalReceived, 0);
			if (n <= 0) return false;
			totalReceived += n;
		}
		outPacket.data.assign(dataBuffer.begin(), dataBuffer.end()); // [cite: 16]
	}
	return true;
}