#include "XmlParser.h"

std::string XmlParser::extractTagValue(const std::string& xml, const std::string& tag) {
	std::string startTag = "<" + tag + ">";
	std::string endTag = "</" + tag + ">";

	size_t startPos = xml.find(startTag);
	if (startPos == std::string::npos) {
		// 部分带有属性的复杂标签定位
		size_t attrPos = xml.find("<" + tag + " ");
		if (attrPos != std::string::npos) {
			startPos = xml.find(">", attrPos);
			if (startPos != std::string::npos) startPos += 1;
		}
	}
	else {
		startPos += startTag.length();
	}

	size_t endPos = xml.find(endTag);

	if (startPos == std::string::npos || endPos == std::string::npos || startPos >= endPos) {
		throw std::runtime_error("XML 格式校验失败: 未能识别标签 " + tag); // [cite: 107]
	}

	return xml.substr(startPos, endPos - startPos);
}