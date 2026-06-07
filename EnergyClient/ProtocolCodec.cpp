#include "ProtocolCodec.h"

#include <arpa/inet.h>

std::vector<uint8_t>
ProtocolCodec::encode(
	uint8_t type,
	const std::string& xml)
{
	std::vector<uint8_t> packet;

	uint16_t head = htons(0xCFCF);

	uint32_t len =
		htonl(
			static_cast<uint32_t>(
				xml.size()));

	packet.insert(
		packet.end(),
		reinterpret_cast<uint8_t*>(&head),
		reinterpret_cast<uint8_t*>(&head) + 2);

	packet.push_back(type);

	packet.insert(
		packet.end(),
		reinterpret_cast<uint8_t*>(&len),
		reinterpret_cast<uint8_t*>(&len) + 4);

	packet.insert(
		packet.end(),
		xml.begin(),
		xml.end());

	return packet;
}

//²ð°üÆ÷
bool ProtocolCodec::decode(
	std::vector<uint8_t>& buffer,
	Packet& packet)
{
	if (buffer.size() < 7)
		return false;

	uint16_t head;

	memcpy(
		&head,
		buffer.data(),
		2);

	head = ntohs(head);

	if (head != 0xCFCF)
	{
		buffer.clear();
		return false;
	}

	packet.type = buffer[2];

	uint32_t len;

	memcpy(
		&len,
		buffer.data() + 3,
		4);

	len = ntohl(len);

	if (buffer.size() < len + 7)
		return false;

	packet.xml.assign(
		buffer.begin() + 7,
		buffer.begin() + 7 + len);

	buffer.erase(
		buffer.begin(),
		buffer.begin() + 7 + len);

	return true;
}