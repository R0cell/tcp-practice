#pragma once

#include <string>
#include <functional>

class AuthManager
{
public:

	using Md5Callback =
		std::function<void(
			const std::string& md5)>;

public:

	void setMd5Callback(
		Md5Callback cb);

	void processSequence(
		const std::string& sequence);

	bool isAuthenticated() const;

	void setAuthenticated(
		bool state);

private:

	bool authenticated_ = false;

	Md5Callback md5Callback_;
};