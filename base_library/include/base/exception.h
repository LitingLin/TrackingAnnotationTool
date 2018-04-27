#pragma once
#include <stdexcept>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>

namespace Base {
	enum class ErrorCodeType
	{
		WIN32API, HRESULT, NTSTATUS, CRT, SQLITE3, USERDEFINED
	};

	class FatalError : public std::runtime_error
	{
	public:
		explicit FatalError(const std::string& _Message, int64_t errorCode, ErrorCodeType errorCodeType);
		explicit FatalError(const char* _Message, int64_t errorCode, ErrorCodeType errorCodeType);
		int64_t getErrorCode() const;
		DWORD getErrorCodeAsWinAPI() const;
		HRESULT getErrorCodeAsHRESULT() const;
		NTSTATUS getErrorCodeAsNTSTATUS() const;
		errno_t getErrorCodeAsCRT() const;
		ErrorCodeType getErrorCodeType() const;
	private:
		int64_t errorCode;
		ErrorCodeType errorCodeType;
	};

	class RuntimeException : public std::runtime_error
	{
	public:
		explicit RuntimeException(const std::string& _Message, int64_t errorCode, ErrorCodeType errorCodeType);
		explicit RuntimeException(const char* _Message, int64_t errorCode, ErrorCodeType errorCodeType);
		int64_t getErrorCode() const;
		DWORD getErrorCodeAsWinAPI() const;
		HRESULT getErrorCodeAsHRESULT() const;
		NTSTATUS getErrorCodeAsNTSTATUS() const;
		errno_t getErrorCodeAsCRT() const;
		ErrorCodeType getErrorCodeType() const;
	private:
		int64_t errorCode;
		ErrorCodeType errorCodeType;
	};

	class NetworkAddressInUseException : public RuntimeException
	{
	public:
		NetworkAddressInUseException(const std::string& _Message)
			: RuntimeException(_Message, 0, ErrorCodeType::USERDEFINED)
		{
		}

		NetworkAddressInUseException(const char* _Message)
			: RuntimeException(_Message, 0, ErrorCodeType::USERDEFINED)
		{
		}
	};

	HRESULT getHRESULTFromExceptionType(const FatalError &exp);
	HRESULT getHRESULTFromExceptionType(const RuntimeException &exp);
}

#define RETURN_HRESULT_ON_CAUGHT_EXCEPTION_BEGIN \
try \
{

#define RETURN_HRESULT_ON_CAUGHT_EXCEPTION_END \
} \
catch (Base::RuntimeException &exp) \
{ \
	return Base::getHRESULTFromExceptionType(exp); \
} \
catch (Base::FatalError &exp) \
{ \
return Base::getHRESULTFromExceptionType(exp); \
} \
catch (std::bad_alloc &) \
{ \
return E_OUTOFMEMORY; \
} \
catch (...) \
{ \
return E_FAIL; \
}
