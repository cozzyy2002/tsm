#include "stdafx.h"
#include "MyObject.h"

MyObject::MyObject(LPCTSTR name /*= nullptr*/, ILogger* logger /*= nullptr*/)
	: m_name(name ? name : _T("")), m_logger(logger)
{
}


MyObject::~MyObject()
{
}

LPCTSTR MyObject::toString() const
{
	if(m_string.empty()) {
		std::tostringstream stream;
		if(m_name.empty()) {
			CA2T typeName(typeid(*this).name());
			stream << (LPCTSTR)typeName;
		} else {
			stream << m_name;
		}
		stream << _T("(0x") << std::hex << (void*)this << _T(")");
		m_string = stream.str();
	}
	return m_string.c_str();
}

LPCTSTR namedEnumToString(int value, LPCTSTR vaArgs, std::vector<std::tstring>& names)
{
	if(names.empty()) {
		auto vaArgsLen = _tcslen(vaArgs) + 1;
		std::unique_ptr<TCHAR[]> _vaArgs(new TCHAR[vaArgsLen]);
		CopyMemory(_vaArgs.get(), vaArgs, vaArgsLen);
		TCHAR* nextToken = nullptr;
		TCHAR* token;
		TCHAR* str = _vaArgs.get();
		do {
			token = _tcstok_s(str, _T(", \t"), &nextToken);
			if(token) names.push_back(token);
			str = nullptr;
		} while(token);
	}

	return (value < (int)names.size()) ? names[value].c_str() : _T("<invalid>");
}
