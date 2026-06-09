#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <string>
#include <vector>

class TcpClient {
private:
	int m_socket;
	bool m_connected;
	std::vector<uint8_t> m_readBuffer; // 接收流缓冲区

public:
	TcpClient();
	~TcpClient();

	bool connectToServer(const std::string& ip, int port);
	void disconnect();

	// 发送原生字节流
	bool sendRaw(const std::vector<uint8_t>& data);

	// 物理读取网络数据，追加到流缓冲区中
	bool readToBuffer();

	// 暴露流缓冲区供上层 Codec 处理
	std::vector<uint8_t>& getBuffer() { return m_readBuffer; }
	bool isConnected() const { return m_connected; }
};

#endif // TCP_CLIENT_H