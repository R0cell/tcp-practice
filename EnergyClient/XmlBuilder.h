#ifndef XML_BUILDER_H
#define XML_BUILDER_H

#include <string>
#include <vector>

struct EnergyItem {
	std::string code;
	std::string value;
};

class XmlBuilder {
public:
	static std::string buildAuthRequest(const std::string& bId, const std::string& gId); // [cite: 42]
	static std::string buildAuthMd5(const std::string& bId, const std::string& gId, const std::string& md5); // [cite: 46]
	static std::string buildHeartbeat(const std::string& bId, const std::string& gId, const std::string& timeStr); // [cite: 63]
	static std::string buildEnergyReport(const std::string& bId, const std::string& gId, const std::string& timeStr, const std::vector<EnergyItem>& items); // [cite: 68, 77]
};

#endif // XML_BUILDER_H