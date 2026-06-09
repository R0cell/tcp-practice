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
	//Windows系统的网络架构和Linux/Unix不同，Linux默认就可以直接调用Socket函数，而Windows要求你在使用任何网络功能
	//（比如建立TCP连接、发送数据）之前，必须先“激活”网络库。
	//初始化Windows网络库（Winsock）并获取其版本信息的配置表
#if defined(_WIN32)
	WSADATA wsaData;
	//申请使用 2.2 版本的 Winsock 协议栈
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
	//让网络根据前两个参数，自动选择默认的协议（Default Protocol）
	m_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_socket < 0) return false;

	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	//htons：所有在网络上传输的多字节数据，必须统一使用大端序，
	//这被称为网络字节序（Network Byte Order)。
	//htons：网络端口号（Port）的范围是0 ~ 65535，
	//在C / C++中，unsigned short恰好是16位（2个字节），完美对应端口号的范围
	serverAddr.sin_port = htons(port);
	//inet_pton：将互联网地址从字符串格式转换为二进制数值格式。
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
	//send第二个参数：硬性规定必须是const char*（字符指针）， reinterpret_cast自动化的类型转换
	//send第四个参数：用于控制网络发送行为，0代表默认；
	//阻塞模式下：如果蓝牙的串口满了，send函数会一直卡在那里等待，直到有空间把数据写进去才返回。
	//阻塞非模式下：有多少空间就发多少，不进行特殊处理。
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
		//.assign()重新分配/覆盖分配
		outPacket.data.assign(dataBuffer.begin(), dataBuffer.end()); // [cite: 16]
	}
	return true;
}