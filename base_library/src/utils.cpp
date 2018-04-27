#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <Objbase.h>
#include <string>

#include <base/logging.h>
#include <random>

#pragma comment(lib, "Rpcrt4.lib")

bool operator<(const GUID& left, const GUID& right)
{
	return strncmp((const char*)&left, (const char*)&right, sizeof(GUID)) < 0;
}

namespace Base
{
	std::wstring UTF8ToUTF16(const std::string &str)
	{
		if (str.empty())
			return std::wstring();
		int str_size;
		if (str.size() > size_t(std::numeric_limits<int>::max()))
			str_size = std::numeric_limits<int>::max();
		else
			str_size = int(str.size());
		int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str_size, NULL, NULL);
		std::wstring wideCharString;
		wideCharString.resize(size);
		CHECK_NE_WIN32API(MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str_size, &wideCharString[0], size + 1), 0);
		return wideCharString;
	}

	std::string UTF16ToUTF8(const std::wstring &str)
	{
		if (str.empty())
			return std::string();
		int str_size;
		if (str.size() > size_t(std::numeric_limits<int>::max()))
			str_size = std::numeric_limits<int>::max();
		else
			str_size = int(str.size());
		int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str_size, nullptr, 0, NULL, NULL);
		std::string localMultiByteString;
		localMultiByteString.resize(size);
		CHECK_NE_WIN32API(WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str_size, &localMultiByteString[0], size + 1, NULL, NULL), 0);
		return localMultiByteString;
	}

	std::wstring toLowerCase(const std::wstring& str)
	{
		std::wstring lowercaseString = str;

		CharLowerW((wchar_t*)lowercaseString.data());

		return lowercaseString;
	}

	bool isMemoryZero(void *buf, const size_t size)
	{
		CHECK(size);
		char *buf_ = (char*)buf;
		return buf_[0] == 0 && memcmp(buf_, buf_ + 1, size - 1) == 0;
	}

	void generateGUID(GUID *guid)
	{
		UuidCreate(guid);
	}

	void generateGUID(wchar_t *str, unsigned str_size)
	{
		GUID guid;
		UuidCreate(&guid);
		GUIDToString(&guid, str, str_size);
	}

	std::wstring generateGUID()
	{
		std::wstring guidString;
		guidString.resize(GUID_STRING_SIZE);
		GUID guid;
		UuidCreate(&guid);
		StringFromGUID2(guid, (wchar_t*)guidString.data(), GUID_STRING_SIZE);

		return guidString;
	}

	void GUIDToString(const GUID *guid, wchar_t *str, unsigned str_size)
	{
		ENSURE_GE(str_size, 39U);
		ENSURE_EQ(StringFromGUID2(*guid, str, 39), 39);
	}

	bool isRunningOn64bitSystem()
	{
#ifdef _M_X64
		return true;
#else
		BOOL isWow64Process;
		if (!IsWow64Process(GetCurrentProcess(), &isWow64Process))
			return false;
		return isWow64Process;
#endif
	}

	bool StringToGUID(const wchar_t *str, unsigned str_size, GUID *guid)
	{
		ENSURE_GE(str_size, 39U);
		HRESULT hr = IIDFromString(str, guid);
		if (SUCCEEDED(hr))
			return true;
		else
			return false;
	}

	uint8_t generateRandomUint8(uint8_t from, uint8_t to)
	{
		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<uint16_t> dis(from, to);
		return uint8_t(dis(gen));
	}

	uint16_t generateRandomUint16(uint16_t from, uint16_t to)
	{
		std::random_device rd;  //Will be used to obtain a seed for the random number engine
		std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
		std::uniform_int_distribution<uint16_t> dis(from, to);
		return dis(gen);
	}
}