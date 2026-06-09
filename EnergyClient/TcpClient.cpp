#include "TcpClient.h"
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
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

bool TcpClient::sendRaw(const std::vector<uint8_t>& data) {
	if (!m_connected) return false;
	int ret = send(m_socket, reinterpret_cast<const char*>(data.data()), data.size(), 0);
	return ret == static_cast<int>(data.size());
}

bool TcpClient::readToBuffer() {
	if (!m_connected) return false;

	char tempBuf[4096];
	int n = recv(m_socket, tempBuf, sizeof(tempBuf), 0);
	if (n > 0) {
		m_readBuffer.insert(m_readBuffer.end(), tempBuf, tempBuf + n);
		return true;
	}
	else if (n == 0) {
		m_connected = false; // ¶Ô·½¶Ï¿ª
	}
	return false;
}