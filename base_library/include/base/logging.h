#pragma once

#include "exception.h"
#include "utils.h"

#include <locale>
#include <functional>
#include <codecvt>
#include <memory>
#include <sstream>

#define SPDLOG_WCHAR_FILENAMES
#include <spdlog/spdlog.h>

extern std::shared_ptr<spdlog::logger> logger;
namespace Base
{

	std::string getStackTrace();
	class Win32ErrorCodeToString
	{
	public:
		Win32ErrorCodeToString(unsigned long errorCode, ...);
		std::string getString() const;
		std::wstring getWString() const;
		~Win32ErrorCodeToString();
	private:
		mutable std::string ansi_str;
		wchar_t *str;
	};

	std::string getWin32LastErrorString();
	std::string getWin32ErrorString(DWORD errorCode);
	std::wstring getHRESULTErrorWString(HRESULT hr);
	std::string getHRESULTErrorString(HRESULT hr);
	std::wstring getNtStatusErrorWString(NTSTATUS ntstatus);
	std::string getNtStatusErrorString(NTSTATUS ntStatus);
	std::string getCRTErrorString(errno_t errnum);

	class FatalErrorLogging
	{
	public:
		FatalErrorLogging(ErrorCodeType errorCodeType, int64_t errorcode, const char* file, int line, const char* function);
		FatalErrorLogging(ErrorCodeType errorCodeType, int64_t errorcode, const char* file, int line, const char* function, const char* exp);
		FatalErrorLogging(ErrorCodeType errorCodeType, int64_t errorcode, const char* file, int line, const char* function, const char* exp1, const char* op, const char* exp2);
		~FatalErrorLogging() noexcept(false);
		std::ostringstream& stream();
	private:
		int64_t errorcode;
		ErrorCodeType errorCodeType;
		std::ostringstream str_stream;
	};

	class RuntimeExceptionLogging
	{
	public:
		RuntimeExceptionLogging(ErrorCodeType errorCodeType, int64_t errorcode, const char* file, int line, const char* function);
		RuntimeExceptionLogging(ErrorCodeType errorCodeType, int64_t errorcode, const char* file, int line, const char* function, const char* exp);
		RuntimeExceptionLogging(ErrorCodeType errorCodeType, int64_t errorcode, const char* file, int line, const char* function, const char* exp1, const char* op, const char* exp2);
		~RuntimeExceptionLogging() noexcept(false);
		std::ostringstream& stream();
	private:
		int64_t errorcode;
		ErrorCodeType errorCodeType;
		std::ostringstream str_stream;
	};

	class EventLogging
	{
	public:
		EventLogging(const char* file, int line, const char* function);
		EventLogging(const char* file, int line, const char* function, const char* exp);
		EventLogging(const char* file, int line, const char* function, const char* exp1, const char* op, const char* exp2);
		~EventLogging() noexcept(false);
		std::ostringstream& stream();
	private:
		std::ostringstream str_stream;
	};

	template <typename T1, typename T2, typename Op>
	std::unique_ptr<std::pair<T1, T2>> check_impl(const T1 &a, const T2 &b, Op op) {
		if (op(a, b))
			return nullptr;
		else
			return std::make_unique<std::pair<T1, T2>>(a, b);
	}
	HRESULT getHRESULTFromRuntimeException(const RuntimeException&exp);
	HRESULT getHRESULTFromFatalError(const FatalError&exp);
}

#define LEFT_OPERAND_RC \
	_rc->first
#define RIGHT_OPERAND_RC \
	_rc->second

// Let it crash, don't catch
#define FATAL_ERROR \
Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__).stream()

#define FATAL_ERROR_WITH_CODE(errorcode) \
Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, errorcode, __FILE__, __LINE__, __func__).stream()

#define ENSURE(val) \
	if (!(val)) \
Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__, #val).stream()

#define ENSURE_WITH_CODE(val, errorcode) \
	if (!(val)) \
Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, errorcode, __FILE__, __LINE__, __func__, #val).stream()

#define ENSURE_WIN32API(val) \
	if (!(val)) \
Base::FatalErrorLogging(Base::ErrorCodeType::WIN32API, GetLastError(), __FILE__, __LINE__, __func__, #val).stream() << Base::getWin32LastErrorString()

#define ENSURE_HR(exp) \
	if (HRESULT _hr = (exp)) if (_hr < 0) \
Base::FatalErrorLogging(Base::ErrorCodeType::HRESULT, _hr, __FILE__, __LINE__, __func__, #exp).stream() << Base::getHRESULTErrorString(_hr)

#define ENSURE_NTSTATUS(val) \
	if (NTSTATUS _status = (val)) if (_status < 0) \
Base::FatalErrorLogging(Base::ErrorCodeType::NTSTATUS, _status, __FILE__, __LINE__, __func__, #val).stream() << Base::getNtStatusErrorString(_status)

#define ENSURE_CRTAPI(val) \
	if (val) \
Base::FatalErrorLogging(Base::ErrorCodeType::CRT, errno, __FILE__, __LINE__, __func__, #val).stream() << Base::getCRTErrorString(errno)

#define ENSURE_OP(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") "

#define ENSURE_OP_WITH_CODE(exp1, exp2, op, functional_op, errorcode)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, errorcode, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") "

#define ENSURE_OP_WIN32API(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::FatalErrorLogging(Base::ErrorCodeType::WIN32API, GetLastError(), __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") " << Base::getWin32LastErrorString()

#define ENSURE_OP_CRTAPI(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::FatalErrorLogging(Base::ErrorCodeType::CRT, errno, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") " << Base::getCRTErrorString(errno)

#define ENSURE_EQ(exp1, exp2) \
	ENSURE_OP(exp1, exp2, ==, std::equal_to<>())
#define ENSURE_NE(exp1, exp2) \
	ENSURE_OP(exp1, exp2, !=, std::not_equal_to<>())
#define ENSURE_LE(exp1, exp2) \
	ENSURE_OP(exp1, exp2, <=, std::less_equal<>())
#define ENSURE_LT(exp1, exp2) \
	ENSURE_OP(exp1, exp2, <, std::less<>())
#define ENSURE_GE(exp1, exp2) \
	ENSURE_OP(exp1, exp2, >=, std::greater_equal<>())
#define ENSURE_GT(exp1, exp2) \
	ENSURE_OP(exp1, exp2, >, std::greater<>())

#define ENSURE_EQ_WITH_CODE(exp1, exp2, errorcode) \
	ENSURE_OP_WITH_CODE(exp1, exp2, ==, std::equal_to<>(), errorcode)
#define ENSURE_NE_WITH_CODE(exp1, exp2, errorcode) \
	ENSURE_OP_WITH_CODE(exp1, exp2, !=, std::not_equal_to<>(), errorcode)
#define ENSURE_LE_WITH_CODE(exp1, exp2, errorcode) \
	ENSURE_OP_WITH_CODE(exp1, exp2, <=, std::less_equal<>(), errorcode)
#define ENSURE_LT_WITH_CODE(exp1, exp2, errorcode) \
	ENSURE_OP_WITH_CODE(exp1, exp2, <, std::less<>(), errorcode)
#define ENSURE_GE_WITH_CODE(exp1, exp2, errorcode) \
	ENSURE_OP_WITH_CODE(exp1, exp2, >=, std::greater_equal<>(), errorcode)
#define ENSURE_GT_WITH_CODE(exp1, exp2, errorcode) \
	ENSURE_OP_WITH_CODE(exp1, exp2, >, std::greater<>(), errorcode)

#define ENSURE_EQ_WIN32API(exp1, exp2) \
	ENSURE_OP_WIN32API(exp1, exp2, ==, std::equal_to<>())
#define ENSURE_NE_WIN32API(exp1, exp2) \
	ENSURE_OP_WIN32API(exp1, exp2, !=, std::not_equal_to<>())
#define ENSURE_LE_WIN32API(exp1, exp2) \
	ENSURE_OP_WIN32API(exp1, exp2, <=, std::less_equal<>())
#define ENSURE_LT_WIN32API(exp1, exp2) \
	ENSURE_OP_WIN32API(exp1, exp2, <, std::less<>())
#define ENSURE_GE_WIN32API(exp1, exp2) \
	ENSURE_OP_WIN32API(exp1, exp2, >=, std::greater_equal<>())
#define ENSURE_GT_WIN32API(exp1, exp2) \
	ENSURE_OP_WIN32API(exp1, exp2, >, std::greater<>())

#define ENSURE_EQ_CRTAPI(exp1, exp2) \
	ENSURE_OP_CRTAPI(exp1, exp2, ==, std::equal_to<>())
#define ENSURE_NE_CRTAPI(exp1, exp2) \
	ENSURE_OP_CRTAPI(exp1, exp2, !=, std::not_equal_to<>())
#define ENSURE_LE_CRTAPI(exp1, exp2) \
	ENSURE_OP_CRTAPI(exp1, exp2, <=, std::less_equal<>())
#define ENSURE_LT_CRTAPI(exp1, exp2) \
	ENSURE_OP_CRTAPI(exp1, exp2, <, std::less<>())
#define ENSURE_GE_CRTAPI(exp1, exp2) \
	ENSURE_OP_CRTAPI(exp1, exp2, >=, std::greater_equal<>())
#define ENSURE_GT_CRTAPI(exp1, exp2) \
	ENSURE_OP_CRTAPI(exp1, exp2, >, std::greater<>())

#define NOT_IMPLEMENTED_ERROR \
	Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__).stream() << "Unknown internal error. "

#define UNREACHABLE_ERROR \
	Base::FatalErrorLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__).stream() << "Unknown internal error. "

// Normal error
#define CHECK(val) \
	if (!(val)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__, #val).stream()

#define CHECK_WITH_CODE(val, errorcode) \
	if (!(val)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::USERDEFINED, errorcode, __FILE__, __LINE__, __func__, #val).stream()

#define CHECK_WIN32API(val) \
	if (!(val)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::WIN32API, GetLastError(), __FILE__, __LINE__, __func__, #val).stream() << Base::getWin32LastErrorString()

#define CHECK_HR(exp) \
	if (HRESULT _hr = (exp)) if (_hr < 0) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::HRESULT, _hr, __FILE__, __LINE__, __func__, #exp).stream() << Base::getHRESULTErrorString(_hr)

#define CHECK_NTSTATUS(val) \
	if (NTSTATUS _status = (val)) if (_status < 0) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::NTSTATUS, _status, __FILE__, __LINE__, __func__, #val).stream() << Base::getNtStatusErrorString(_status)

#define CHECK_CRTAPI(val) \
	if (val) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::CRT, errno, __FILE__, __LINE__, __func__, #val).stream() << Base::getCRTErrorString(errno)

#define CHECK_OP(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") "

#define CHECK_OP_WITH_CODE(exp1, exp2, op, functional_op, errorcode)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::USERDEFINED, errorcode, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") "

#define CHECK_OP_WITH_CODE_WIN32API(exp1, exp2, op, functional_op, errorcode)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::WIN32API, errorcode, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") "  << getWin32ErrorString(errorcode)

#define CHECK_OP_HR(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::HRESULT, LEFT_OPERAND_RC, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") "  << Base::getHRESULTErrorString(LEFT_OPERAND_RC)

#define CHECK_OP_WIN32API(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::WIN32API, GetLastError(), __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") " << Base::getWin32LastErrorString()

#define CHECK_OP_CRTAPI(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::RuntimeExceptionLogging(Base::ErrorCodeType::CRT, errno, __FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") " << Base::getCRTErrorString(errno)

#define CHECK_EQ(exp1, exp2) \
	CHECK_OP(exp1, exp2, ==, std::equal_to<>())
#define CHECK_NE(exp1, exp2) \
	CHECK_OP(exp1, exp2, !=, std::not_equal_to<>())
#define CHECK_LE(exp1, exp2) \
	CHECK_OP(exp1, exp2, <=, std::less_equal<>())
#define CHECK_LT(exp1, exp2) \
	CHECK_OP(exp1, exp2, <, std::less<>())
#define CHECK_GE(exp1, exp2) \
	CHECK_OP(exp1, exp2, >=, std::greater_equal<>())
#define CHECK_GT(exp1, exp2) \
	CHECK_OP(exp1, exp2, >, std::greater<>())

#define CHECK_EQ_WITH_CODE(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE(exp1, exp2, ==, std::equal_to<>(), errorcode)
#define CHECK_NE_WITH_CODE(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE(exp1, exp2, !=, std::not_equal_to<>(), errorcode)
#define CHECK_LE_WITH_CODE(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE(exp1, exp2, <=, std::less_equal<>(), errorcode)
#define CHECK_LT_WITH_CODE(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE(exp1, exp2, <, std::less<>(), errorcode)
#define CHECK_GE_WITH_CODE(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE(exp1, exp2, >=, std::greater_equal<>(), errorcode)
#define CHECK_GT_WITH_CODE(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE(exp1, exp2, >, std::greater<>(), errorcode)

#define CHECK_EQ_WITH_CODE_WIN32API(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE_WIN32API(exp1, exp2, ==, std::equal_to<>(), errorcode)
#define CHECK_NE_WITH_CODE_WIN32API(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE_WIN32API(exp1, exp2, !=, std::not_equal_to<>(), errorcode)
#define CHECK_LE_WITH_CODE_WIN32API(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE_WIN32API(exp1, exp2, <=, std::less_equal<>(), errorcode)
#define CHECK_LT_WITH_CODE_WIN32API(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE_WIN32API(exp1, exp2, <, std::less<>(), errorcode)
#define CHECK_GE_WITH_CODE_WIN32API(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE_WIN32API(exp1, exp2, >=, std::greater_equal<>(), errorcode)
#define CHECK_GT_WITH_CODE_WIN32API(exp1, exp2, errorcode) \
	CHECK_OP_WITH_CODE_WIN32API(exp1, exp2, >, std::greater<>(), errorcode)

#define CHECK_EQ_HR(exp1, exp2) \
	CHECK_OP_HR(exp1, exp2, ==, std::equal_to<>())
#define CHECK_NE_HR(exp1, exp2) \
	CHECK_OP_HR(exp1, exp2, !=, std::not_equal_to<>())

#define CHECK_EQ_WIN32API(exp1, exp2) \
	CHECK_OP_WIN32API(exp1, exp2, ==, std::equal_to<>())
#define CHECK_NE_WIN32API(exp1, exp2) \
	CHECK_OP_WIN32API(exp1, exp2, !=, std::not_equal_to<>())
#define CHECK_LE_WIN32API(exp1, exp2) \
	CHECK_OP_WIN32API(exp1, exp2, <=, std::less_equal<>())
#define CHECK_LT_WIN32API(exp1, exp2) \
	CHECK_OP_WIN32API(exp1, exp2, <, std::less<>())
#define CHECK_GE_WIN32API(exp1, exp2) \
	CHECK_OP_WIN32API(exp1, exp2, >=, std::greater_equal<>())
#define CHECK_GT_WIN32API(exp1, exp2) \
	CHECK_OP_WIN32API(exp1, exp2, >, std::greater<>())

#define CHECK_EQ_CRTAPI(exp1, exp2) \
	CHECK_OP_CRTAPI(exp1, exp2, ==, std::equal_to<>())
#define CHECK_NE_CRTAPI(exp1, exp2) \
	CHECK_OP_CRTAPI(exp1, exp2, !=, std::not_equal_to<>())
#define CHECK_LE_CRTAPI(exp1, exp2) \
	CHECK_OP_CRTAPI(exp1, exp2, <=, std::less_equal<>())
#define CHECK_LT_CRTAPI(exp1, exp2) \
	CHECK_OP_CRTAPI(exp1, exp2, <, std::less<>())
#define CHECK_GE_CRTAPI(exp1, exp2) \
	CHECK_OP_CRTAPI(exp1, exp2, >=, std::greater_equal<>())
#define CHECK_GT_CRTAPI(exp1, exp2) \
	CHECK_OP_CRTAPI(exp1, exp2, >, std::greater<>())

#define NOT_EXPECT_EXCEPTION \
	Base::RuntimeExceptionLogging(Base::ErrorCodeType::USERDEFINED, -1, __FILE__, __LINE__, __func__).stream() << "Unexpected parameter."

// Just logging
#define LOG_IF_FAILED(val) \
	if (!(val)) \
Base::EventLogging(__FILE__, __LINE__, __func__, #val).stream()

#define LOG_IF_FAILED_WIN32API(val) \
	if (!(val)) \
Base::EventLogging(__FILE__, __LINE__, __func__, #val).stream() << Base::getWin32LastErrorString()

#define LOG_IF_FAILED_HR(exp) \
	if (HRESULT _hr = (exp)) if (_hr < 0) \
Base::EventLogging(__FILE__, __LINE__, __func__, #exp).stream() << Base::getHRESULTErrorString(_hr)

#define LOG_IF_FAILED_NTSTATUS(val) \
	if (NTSTATUS _status = (val)) if (_status < 0) \
Base::EventLogging(__FILE__, __LINE__, __func__, #val).stream() << Base::getNtStatusErrorString(_status)

#define LOG_IF_FAILED_CRTAPI(val) \
	if (val) \
Base::EventLogging(__FILE__, __LINE__, __func__, #val).stream() << Base::getCRTErrorString(errno)

#define LOG_IF_OP(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::EventLogging(__FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") "

#define LOG_IF_OP_WIN32API(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::EventLogging(__FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") " << Base::getWin32LastErrorString()

#define LOG_IF_OP_CRTAPI(exp1, exp2, op, functional_op)  \
	if (auto _rc = Base::check_impl((exp1), (exp2), functional_op)) \
Base::EventLogging(__FILE__, __LINE__, __func__, #exp1, #op, #exp2).stream() \
	<< '(' << LEFT_OPERAND_RC << " vs. " << RIGHT_OPERAND_RC << ") " << Base::getCRTErrorString(errno)

#define LOG_IF_NOT_EQ(exp1, exp2) \
	LOG_IF_OP(exp1, exp2, ==, std::equal_to<>())
#define LOG_IF_NOT_NE(exp1, exp2) \
	LOG_IF_OP(exp1, exp2, !=, std::not_equal_to<>())
#define LOG_IF_NOT_LE(exp1, exp2) \
	LOG_IF_OP(exp1, exp2, <=, std::less_equal<>())
#define LOG_IF_NOT_LT(exp1, exp2) \
	LOG_IF_OP(exp1, exp2, <, std::less<>())
#define LOG_IF_NOT_GE(exp1, exp2) \
	LOG_IF_OP(exp1, exp2, >=, std::greater_equal<>())
#define LOG_IF_NOT_GT(exp1, exp2) \
	LOG_IF_OP(exp1, exp2, >, std::greater<>())

#define LOG_IF_NOT_EQ_WIN32API(exp1, exp2) \
	LOG_IF_OP_WIN32API(exp1, exp2, ==, std::equal_to<>())
#define LOG_IF_NOT_NE_WIN32API(exp1, exp2) \
	LOG_IF_OP_WIN32API(exp1, exp2, !=, std::not_equal_to<>())
#define LOG_IF_NOT_LE_WIN32API(exp1, exp2) \
	LOG_IF_OP_WIN32API(exp1, exp2, <=, std::less_equal<>())
#define LOG_IF_NOT_LT_WIN32API(exp1, exp2) \
	LOG_IF_OP_WIN32API(exp1, exp2, <, std::less<>())
#define LOG_IF_NOT_GE_WIN32API(exp1, exp2) \
	LOG_IF_OP_WIN32API(exp1, exp2, >=, std::greater_equal<>())
#define LOG_IF_NOT_GT_WIN32API(exp1, exp2) \
	LOG_IF_OP_WIN32API(exp1, exp2, >, std::greater<>())

#define LOG_IF_NOT_EQ_CRTAPI(exp1, exp2) \
	LOG_IF_OP_CRTAPI(exp1, exp2, ==, std::equal_to<>())
#define LOG_IF_NOT_NE_CRTAPI(exp1, exp2) \
	LOG_IF_OP_CRTAPI(exp1, exp2, !=, std::not_equal_to<>())
#define LOG_IF_NOT_LE_CRTAPI(exp1, exp2) \
	LOG_IF_OP_CRTAPI(exp1, exp2, <=, std::less_equal<>())
#define LOG_IF_NOT_LT_CRTAPI(exp1, exp2) \
	LOG_IF_OP_CRTAPI(exp1, exp2, <, std::less<>())
#define LOG_IF_NOT_GE_CRTAPI(exp1, exp2) \
	LOG_IF_OP_CRTAPI(exp1, exp2, >=, std::greater_equal<>())
#define LOG_IF_NOT_GT_CRTAPI(exp1, exp2) \
	LOG_IF_OP_CRTAPI(exp1, exp2, >, std::greater<>())
