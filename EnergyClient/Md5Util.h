#pragma once

#include <string>

class Md5Util
{
public:

	static std::string md5(
		const std::string& text);
};

#ifndef MD5_UTIL_H
#define MD5_UTIL_H

#include <string>

class Md5Util {
public:
	// 计算 32 位大写 MD5 值
	static std::string compute(const std::string& input);
};

#endif // MD5_UTIL_H