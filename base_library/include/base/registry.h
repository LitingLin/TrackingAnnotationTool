#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "event.h"

namespace Base
{
	class Registry
	{
	public:
		Registry(const wchar_t *prefix, HKEY rootKey = HKEY_CURRENT_USER, bool wow64Redirection = false);
		void setWow64Redirection(bool enable);
		Registry createSubKey(const wchar_t *subkey) const;
		void create() const;
		bool exist() const;
		bool exist(const wchar_t *name) const;
		std::vector<std::wstring> getSubKeys() const;
		std::vector<std::wstring> getValueNames() const;
		void setDWORD(const wchar_t *name, uint32_t value) const;
		void setQWORD(const wchar_t *name, uint64_t value) const;
		void setString(const wchar_t *name, const wchar_t *str, uint32_t strLenInCharacters) const;
		void setString(const wchar_t *name, const std::wstring &str) const;
		void setBlob(const wchar_t *name, const std::vector<unsigned char> &blob) const;
		bool getDWORD(const wchar_t *name, uint32_t *value) const;
		bool getQWORD(const wchar_t *name, uint64_t *value) const;
		bool getString(const wchar_t *name, std::wstring &str) const;
		bool getStringSize(const wchar_t* name, uint32_t *strLenInCharacters) const;
		bool getBlob(const wchar_t *name, std::vector<unsigned char> &blob) const;
		bool remove() const;
		bool removeKey(const wchar_t *name) const;
		bool remove(const wchar_t *name) const;
		bool saveToFile(const std::wstring &fileName) const;
		Event getEvent() const;
	protected:
		std::wstring prefix;
	private:
		HKEY rootKey;
		bool wow64Redirection;
	};

	class RegistryLocalMachineSoftware : public Registry
	{
	public:
		RegistryLocalMachineSoftware(const wchar_t *orgName, const wchar_t *productName = nullptr, const wchar_t *subkey = nullptr);
	};
}