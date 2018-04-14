#pragma once

class MyObject
{
public:
	MyObject(LPCTSTR name = nullptr);
	virtual ~MyObject();

	virtual LPCTSTR toString() const;

protected:
	const std::tstring m_name;
	mutable std::tstring m_string;
};

#define ENUM(c, ...) \
	class c			\
	{				\
	public:			\
		enum class Values { __VA_ARGS__, _COUNT };	\
		LPCTSTR toString() const {	\
			return namedEnumToString(m_value, _T(#__VA_ARGS__), m_valueNames); \
		}							\
		Values getValue() const { return m_value; }	\
		bool isValid() const { return isValid(m_value); }	\
		static bool isValid(int v) { return (v < (int)Values::_COUNT); }	\
	protected:						\
		mutable std::vector<std::tstring> m_valueNames;		\
		Values m_value;							\
	}

extern LPCTSTR namedEnumToString(int value, LPCTSTR vaArgs, std::vector<std::tstring>& names);

template<typename E, LPCTSTR V>
class NamedEnum
{
public:
	typename E values;
};
