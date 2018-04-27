#include "base/registry.h"
#include "base/logging.h"

#include <Synchapi.h>

namespace Base
{
	class HKEYGuard
	{
	public:
		HKEYGuard(HKEY key, const wchar_t *subkey, bool wow64Redirection)
		{
			if (wow64Redirection)
			{
				CHECK_EQ_WITH_CODE_WIN32API(RegCreateKeyEx(key, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
					NULL, &keyHandle, nullptr),
					ERROR_SUCCESS, LEFT_OPERAND_RC);
			}
			else
			{
				CHECK_EQ_WITH_CODE_WIN32API(RegCreateKeyEx(key, subkey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_64KEY,
					NULL, &keyHandle, nullptr),
					ERROR_SUCCESS, LEFT_OPERAND_RC);
			}
		}
		~HKEYGuard()
		{
			CHECK_EQ_WITH_CODE_WIN32API(RegCloseKey(keyHandle), ERROR_SUCCESS, LEFT_OPERAND_RC);
		}
		HKEY get() const
		{
			return keyHandle;
		}
		operator HKEY() const
		{
			return keyHandle;
		}
	private:
		HKEY keyHandle;
	};

	class HKEYGuard_ReadOnly
	{
	public:
		HKEYGuard_ReadOnly(HKEY key, const wchar_t *subkey, bool wow64Redirection)
		{
			if (wow64Redirection)
			{
				if (RegOpenKeyEx(key, subkey, 0, KEY_READ, &keyHandle) != ERROR_SUCCESS)
					keyHandle = nullptr;
			}
			else
			{
				if (RegOpenKeyEx(key, subkey, 0, KEY_READ | KEY_WOW64_64KEY, &keyHandle) != ERROR_SUCCESS)
					keyHandle = nullptr;
			}
		}
		~HKEYGuard_ReadOnly()
		{
			if (keyHandle != nullptr)
				CHECK_EQ_WITH_CODE_WIN32API(RegCloseKey(keyHandle), ERROR_SUCCESS, LEFT_OPERAND_RC);
		}
		bool is_open()
		{
			return keyHandle != nullptr;
		}
		HKEY get() const
		{
			return keyHandle;
		}
		operator HKEY() const
		{
			return keyHandle;
		}
	private:
		HKEY keyHandle;
	};

	class HKEYGuard_OpenExisting
	{
	public:
		HKEYGuard_OpenExisting(HKEY key, const wchar_t *subkey, bool wow64Redirection)
		{
			if (wow64Redirection)
			{
				if (RegOpenKeyEx(key, subkey, 0, KEY_ALL_ACCESS | DELETE, &keyHandle) != ERROR_SUCCESS)
					keyHandle = nullptr;
			}
			else
			{
				if (RegOpenKeyEx(key, subkey, 0, KEY_ALL_ACCESS | DELETE | KEY_WOW64_64KEY, &keyHandle) != ERROR_SUCCESS)
					keyHandle = nullptr;
			}
		}
		~HKEYGuard_OpenExisting()
		{
			if (keyHandle != nullptr)
				CHECK_EQ_WITH_CODE_WIN32API(RegCloseKey(keyHandle), ERROR_SUCCESS, LEFT_OPERAND_RC);
		}
		bool is_open()
		{
			return keyHandle != nullptr;
		}
		HKEY get() const
		{
			return keyHandle;
		}
		operator HKEY() const
		{
			return keyHandle;
		}
	private:
		HKEY keyHandle;
	};

	RegistryLocalMachineSoftware::RegistryLocalMachineSoftware(const wchar_t* orgName, const wchar_t* productName,
		const wchar_t* subkey)
		: Registry(L"", HKEY_LOCAL_MACHINE)
	{
		prefix = L"SOFTWARE\\";
		if (orgName) {
			prefix += orgName;
			prefix += L'\\';
			if (productName) {
				prefix += productName;
				prefix += L'\\';
				if (subkey) {
					prefix += subkey;
					prefix += L'\\';
				}
			}
		}
	}

	Registry::Registry(const wchar_t* prefix, HKEY rootKey, bool wow64Redirection)
		: prefix(prefix), rootKey(rootKey), wow64Redirection(wow64Redirection)
	{
	}

	void Registry::setWow64Redirection(bool enable)
	{
		wow64Redirection = enable;
	}

	Registry Registry::createSubKey(const wchar_t* subkey) const
	{
		return Registry((prefix + L'\\' + subkey).c_str(), rootKey, wow64Redirection);
	}

	void Registry::create() const
	{
		HKEYGuard keyHandle(rootKey, prefix.c_str(), wow64Redirection);
	}

	bool Registry::getString(const wchar_t* name, std::wstring& str) const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		if (!keyHandle.is_open())
			return false;
		unsigned long strLenInBytes;
		LONG rc = RegQueryValueEx(keyHandle, name, NULL, NULL, NULL, &strLenInBytes);
		if (rc == ERROR_FILE_NOT_FOUND)
			return false;
		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);
		str.resize(strLenInBytes / sizeof(wchar_t) - 1);
		CHECK_EQ_WITH_CODE_WIN32API(RegQueryValueEx(keyHandle, name, NULL, NULL, (LPBYTE)&str[0], &strLenInBytes),
			ERROR_SUCCESS, LEFT_OPERAND_RC);
		return true;
	}

	bool Registry::exist() const
	{
		HKEY keyHANDLE;
		if (RegOpenKeyEx(rootKey, prefix.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &keyHANDLE) == ERROR_SUCCESS)
		{
			CHECK_EQ_WITH_CODE_WIN32API(RegCloseKey(keyHANDLE), ERROR_SUCCESS, LEFT_OPERAND_RC);
			return true;
		}
		else
			return false;
	}

	bool Registry::exist(const wchar_t* name) const
	{
		HKEY keyHANDLE;
		if (RegOpenKeyEx(rootKey, prefix.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &keyHANDLE) == ERROR_SUCCESS)
		{
			LONG rc = RegQueryValueEx(keyHANDLE, name, nullptr, NULL, NULL, NULL);
			CHECK_EQ_WITH_CODE_WIN32API(RegCloseKey(keyHANDLE), ERROR_SUCCESS, LEFT_OPERAND_RC);
			if (rc != ERROR_SUCCESS)
				return false;
			return true;
		}
		else
			return false;
	}

	std::vector<std::wstring> Registry::getSubKeys() const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		DWORD numberOfSubkeys, maxLengthOfSubkey;
		std::vector<std::wstring> subkeys;
		if (!keyHandle.is_open())
			return subkeys;
		if (RegQueryInfoKey(keyHandle, NULL, NULL, NULL, &numberOfSubkeys, &maxLengthOfSubkey, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
			return subkeys;

		subkeys.reserve(numberOfSubkeys);
		std::wstring buffer;
		buffer.resize(maxLengthOfSubkey);
		for (DWORD i = 0; i < numberOfSubkeys; ++i)
		{
			DWORD subkeyLength = maxLengthOfSubkey + 1;
			LONG rc = RegEnumKeyEx(keyHandle, i, &buffer[0], &subkeyLength, NULL, NULL, NULL, NULL);

			if (rc == ERROR_SUCCESS) {
				subkeys.push_back(buffer.substr(0, subkeyLength));
			}
			else if (rc == ERROR_MORE_DATA)
				continue;
			else
				break;
		}
		return subkeys;
	}

	std::vector<std::wstring> Registry::getValueNames() const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		DWORD numberOfValues, maxLengthOfValueName;
		std::vector<std::wstring> valueNames;
		if (!keyHandle.is_open())
			return valueNames;
		if (RegQueryInfoKey(keyHandle, NULL, NULL, NULL, NULL, NULL, NULL, &numberOfValues, &maxLengthOfValueName, NULL, NULL, NULL) != ERROR_SUCCESS)
			return valueNames;

		valueNames.reserve(numberOfValues);
		std::wstring buffer;
		buffer.resize(maxLengthOfValueName);
		for (DWORD i = 0; i < numberOfValues; ++i)
		{
			DWORD subkeyLength = maxLengthOfValueName + 1;
			LONG rc = RegEnumValue(keyHandle, i, &buffer[0], &subkeyLength, NULL, NULL, NULL, NULL);

			if (rc == ERROR_SUCCESS) {
				valueNames.push_back(buffer.substr(0, subkeyLength));
			}
			else if (rc == ERROR_MORE_DATA)
				continue;
			else
				break;
		}
		return valueNames;
	}

	void Registry::setDWORD(const wchar_t* name, uint32_t value) const
	{
		HKEYGuard keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		CHECK_EQ_WITH_CODE_WIN32API(RegSetValueEx(keyHandle, name, 0, REG_DWORD, (const BYTE*)&value, sizeof(uint32_t)),
			ERROR_SUCCESS, LEFT_OPERAND_RC);
	}

	void Registry::setQWORD(const wchar_t* name, uint64_t value) const
	{
		HKEYGuard keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		CHECK_EQ_WITH_CODE_WIN32API(RegSetValueEx(keyHandle, name, 0, REG_QWORD, (const BYTE*)&value, sizeof(uint64_t)),
			ERROR_SUCCESS, LEFT_OPERAND_RC);
	}

	void Registry::setString(const wchar_t* name, const std::wstring& str) const
	{
		setString(name, str.c_str(), (uint32_t)str.size());
	}

	void Registry::setString(const wchar_t* name, const wchar_t* str, uint32_t strLenInCharacters) const
	{
		HKEYGuard keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		CHECK_EQ_WITH_CODE_WIN32API(RegSetValueEx(keyHandle, name, 0, REG_SZ, (const BYTE*)str, (strLenInCharacters + 1) * sizeof(wchar_t)),
			ERROR_SUCCESS, LEFT_OPERAND_RC);
	}

	void Registry::setBlob(const wchar_t* name, const std::vector<unsigned char>& blob) const
	{
		HKEYGuard keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		CHECK_EQ_WITH_CODE_WIN32API(RegSetValueEx(keyHandle, name, 0, REG_BINARY, (const BYTE*)blob.data(), (uint32_t)blob.size()),
			ERROR_SUCCESS, LEFT_OPERAND_RC);
	}

	bool Registry::getDWORD(const wchar_t* name, uint32_t* value) const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		if (!keyHandle.is_open())
			return false;
		DWORD size = sizeof(uint32_t);
		LONG rc = RegGetValue(keyHandle, NULL, name, RRF_RT_REG_DWORD, NULL, value, &size);
		if (rc == ERROR_FILE_NOT_FOUND)
			return false;

		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);

		return true;
	}

	bool Registry::getQWORD(const wchar_t* name, uint64_t* value) const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		if (!keyHandle.is_open())
			return false;
		DWORD size = sizeof(uint64_t);
		LONG rc = RegGetValue(keyHandle, NULL, name, RRF_RT_REG_QWORD, NULL, value, &size);
		if (rc == ERROR_FILE_NOT_FOUND)
			return false;

		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);

		return true;
	}

	bool Registry::getStringSize(const wchar_t* name, uint32_t *strLenInCharacters) const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		if (!keyHandle.is_open())
			return false;
		LONG rc = RegGetValue(keyHandle, NULL, name, RRF_RT_REG_SZ, NULL, NULL, (unsigned long*)strLenInCharacters);
		if (rc == ERROR_FILE_NOT_FOUND || rc == ERROR_MORE_DATA)
			return false;

		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);

		*strLenInCharacters = *strLenInCharacters / sizeof(wchar_t) - 1;
		return true;
	}

	bool Registry::getBlob(const wchar_t* name, std::vector<unsigned char>& blob) const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		if (!keyHandle.is_open())
			return false;
		unsigned long strLenInBytes;
		LONG rc = RegQueryValueEx(keyHandle, name, NULL, NULL, NULL, &strLenInBytes);
		if (rc == ERROR_FILE_NOT_FOUND)
			return false;
		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);
		blob.resize(strLenInBytes);
		CHECK_EQ_WITH_CODE_WIN32API(RegQueryValueEx(keyHandle, name, NULL, NULL, (LPBYTE)&blob[0], &strLenInBytes),
			ERROR_SUCCESS, LEFT_OPERAND_RC);
		return true;
	}

	bool Registry::remove() const
	{
		{
			HKEYGuard_OpenExisting keyHandle(rootKey, prefix.c_str(), wow64Redirection);
			if (!keyHandle.is_open())
				return false;
			const LONG rc = RegDeleteTree(keyHandle, nullptr);
			if (rc == ERROR_FILE_NOT_FOUND)
				return false;

			CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);
		}
		const LONG rc = RegDeleteKeyEx(rootKey, prefix.c_str(), KEY_WOW64_64KEY, 0);
		if (rc == ERROR_FILE_NOT_FOUND)
			return false;

		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);

		return true;
	}

	bool Registry::removeKey(const wchar_t* name) const
	{
		HKEYGuard_OpenExisting keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		if (!keyHandle.is_open())
			return false;
		const LONG rc = RegDeleteTree(keyHandle, name);
		if (rc == ERROR_FILE_NOT_FOUND)
			return false;

		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);
		return true;
	}

	bool Registry::remove(const wchar_t* name) const
	{
		HKEYGuard keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		LONG rc = RegDeleteValue(keyHandle, name);
		if (rc == ERROR_FILE_NOT_FOUND)
			return false;
		CHECK_EQ_WITH_CODE_WIN32API(rc, ERROR_SUCCESS, LEFT_OPERAND_RC);

		return true;
	}

	bool Registry::saveToFile(const std::wstring& fileName) const
	{
		HKEYGuard_ReadOnly keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		if (!keyHandle.is_open())
			return false;
		if (RegSaveKey(keyHandle, fileName.c_str(), NULL) == ERROR_SUCCESS)
			return true;
		else
			return false;
	}

	Event Registry::getEvent() const
	{
		HKEYGuard keyHandle(rootKey, prefix.c_str(), wow64Redirection);
		Event event;
		CHECK_EQ_WITH_CODE_WIN32API(RegNotifyChangeKeyValue(keyHandle, TRUE, REG_NOTIFY_CHANGE_LAST_SET, event.getHandle(), TRUE),
			ERROR_SUCCESS, LEFT_OPERAND_RC);
		return event;
	}
}