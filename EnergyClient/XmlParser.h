#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <string>
#include <stdexcept>

class XmlParser {
public:
	// 解析基础工具，提取XML标签对中的值，带格式异常检查
	static std::string extractTagValue(const std::string& xml, const std::string& tag);
};

#endif // XML_PARSER_H