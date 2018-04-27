#include <base/security_attributes.h>

#include <Sddl.h>
#include <base/logging.h>

namespace Base
{
	AllowAllSecurityATTRIBUTES::AllowAllSecurityATTRIBUTES()
	{
		const WCHAR *pszStringSecurityDescriptor = L"D:(A;;GA;;;WD)(A;;GA;;;AN)S:(ML;;NW;;;ME)";

		ENSURE_WIN32API(ConvertStringSecurityDescriptorToSecurityDescriptor(pszStringSecurityDescriptor, SDDL_REVISION_1, &_descriptor, NULL));

		_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
		_attributes.lpSecurityDescriptor = _descriptor;
		_attributes.bInheritHandle = FALSE;
	}

	AllowAllSecurityATTRIBUTES::~AllowAllSecurityATTRIBUTES()
	{
		LocalFree(_descriptor);
	}

	SECURITY_ATTRIBUTES *AllowAllSecurityATTRIBUTES::get()
	{
		return &_attributes;
	}
}