#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Rpc.h>
#include <string>
#include <limits>

bool operator<(const GUID& left, const GUID& right);
namespace Base
{
	std::wstring UTF8ToUTF16(const std::string &str);
	std::string UTF16ToUTF8(const std::wstring &str);
	//std::string toUpperCase(const std::string &str);
	//std::wstring toUpperCase(const std::wstring &str);
	//std::string toLowerCase(const std::string &str);
	std::wstring toLowerCase(const std::wstring &str);

	bool isMemoryZero(void *buf, const size_t size);

	const uint8_t GUID_STRING_SIZE = 39;
	void generateGUID(GUID *guid);
	void generateGUID(wchar_t *str, unsigned str_size);
	std::wstring generateGUID();
	bool StringToGUID(const wchar_t *str, unsigned str_size, GUID *guid);
	void GUIDToString(const GUID *guid, wchar_t *str, unsigned str_size);
	bool isRunningOn64bitSystem();

	uint8_t generateRandomUint8(uint8_t from = std::numeric_limits<uint8_t>::min(), uint8_t to = std::numeric_limits<uint8_t>::max());
	uint16_t generateRandomUint16(uint16_t from = std::numeric_limits<uint16_t>::min(), uint16_t to = std::numeric_limits<uint16_t>::max());
}