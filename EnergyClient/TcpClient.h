//#pragma once
//
//#include <asio.hpp>
//
//#include <thread>
//#include <vector>
//#include <atomic>
//#include <functional>
//
//class TcpClient
//{
//public:
//
//	using ReceiveCallback =
//		std::function<void(
//			const std::vector<uint8_t>&)>;
//
//public:
//
//	TcpClient();
//
//	~TcpClient();
//
//	bool connect(
//		const std::string& ip,
//		uint16_t port);
//
//	void disconnect();
//
//	bool send(
//		const std::vector<uint8_t>& data);
//
//	bool isConnected() const;
//
//	void setReceiveCallback(
//		ReceiveCallback cb);
//
//private:
//
//	void startRead();
//
//	void handleRead(
//		std::size_t length);
//
//private:
//
//	asio::io_context io_;
//
//	asio::ip::tcp::socket socket_;
//
//	std::thread ioThread_;
//
//	std::atomic<bool> connected_;
//
//	ReceiveCallback receiveCallback_;
//
//	std::vector<uint8_t> recvBuffer_;
//
//	std::array<uint8_t, 4096> tempBuffer_;
//};

#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <vector>
#include "../Protocol/Packet.h"

class TcpClient {
private:
	int m_socket;
	bool m_connected;

public:
	TcpClient();
	~TcpClient();

	bool connectToServer(const std::string& ip, int port);
	void disconnect();
	bool sendPacket(const Packet& packet);
	bool receivePacket(Packet& outPacket);
	bool isConnected() const { return m_connected; }
};

#endif // TCP_CLIENT_H