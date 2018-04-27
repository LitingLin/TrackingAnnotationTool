#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace Base {
	class AllowAllSecurityATTRIBUTES
	{
	public:
		AllowAllSecurityATTRIBUTES();
		~AllowAllSecurityATTRIBUTES();
		SECURITY_ATTRIBUTES *get();
	private:
		PSECURITY_DESCRIPTOR _descriptor;
		SECURITY_ATTRIBUTES _attributes;
	};
}