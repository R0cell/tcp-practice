#include "XmlBuilder.h"

std::string XmlBuilder::buildAuthRequest(const std::string& bId, const std::string& gId) {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root><common><building_id>" + bId +
		"</building_id><gateway_id>" + gId + "</gateway_id><type>id_validate</type></common>"
		"<id_validate operation=\"request\"></id_validate></root>"; // [cite: 42]
}

std::string XmlBuilder::buildAuthMd5(const std::string& bId, const std::string& gId, const std::string& md5) {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root><common><building_id>" + bId +
		"</building_id><gateway_id>" + gId + "</gateway_id><type>id_validate</type></common>"
		"<id_validate operation=\"md5\"><md5>" + md5 + "</md5></id_validate></root>"; // [cite: 46]
}

std::string XmlBuilder::buildHeartbeat(const std::string& bId, const std::string& gId, const std::string& timeStr) {
	return "<?xml version=\"1.0\" encoding=\"UTF-8\"?><root><common><building_id>" + bId +
		"</building_id><gateway_id>" + gId + "</gateway_id><type>heart_beat</type></common>"
		"<heart_beat operation=\"time\"><time>" + timeStr + "</time></heart_beat>"
		"<heart_beat operation=\"notify\"></heart_beat></root>"; // [cite: 63]
}

std::string XmlBuilder::buildEnergyReport(const std::string& bId, const std::string& gId, const std::string& timeStr, const std::vector<EnergyItem>& items) {
	std::string xml = "<?xml version=\"1.0\" encoding=\"utf-8\"?><root><common><building_id>" + bId +
		"</building_id><gateway_id>" + gId + "</gateway_id><type>energy_data</type></common>"
		"<data operation=\"report\"><time>" + timeStr + "</time><energy_items>"; // [cite: 68, 75, 76, 77]
	for (const auto& item : items) {
		xml += "<energy_item code=\"" + item.code + "\">" + item.value + "</energy_item>"; // [cite: 78, 79]
	}
	xml += "</energy_items></data></root>"; // [cite: 80, 89, 90]
	return xml;
}