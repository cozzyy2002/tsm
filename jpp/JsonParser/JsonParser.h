#pragma once

#include <memory>

namespace std {
#ifdef UNICODE
using tstring = wstring;
using tistream = wistream;
using tostream = wostream;
using tistringstream = wistringstream;
using tostringstream = wostringstream;
#else
using tstring = string;
using tistream = istream;
using tostream = ostream;
using tistringstream = istringstream;
using tostringstream = ostringstream;
#endif
} // namespace std

namespace json_parser {

class CJsonParser
{
public:
	CJsonParser();
	~CJsonParser();

	struct Option {
		// Default constructor
		Option() { ZeroMemory(this, sizeof(*this)); }
		// Constructor for remove space.
		Option(bool removeSpace, bool removeEol)
			: removeComment(true), removeSpace(removeSpace), removeEol(removeEol)
			, expandTab(false), tabStop(0) {}
		// Constructor for expand tab.
		Option(unsigned int tabStop, bool removeComment = false, bool removeEol = false)
			: removeComment(removeComment), removeSpace(false), removeEol(removeEol)
			, expandTab(true), tabStop(tabStop) {}

		bool removeComment;		// Remove comment.
		bool removeSpace;		// Remove space(' '), tab('\t')
		bool removeEol;			// Remove CR('\r'), LF('\n')
		bool expandTab;			// Expand tab.
		unsigned int tabStop;	// Tab stop position(Used if expandTab == true).
	};

	HRESULT removeComment(LPCTSTR source, bool preserveEol, std::tstring& out);
	HRESULT preprocess(LPCTSTR source, std::tstring& out, const Option& option);
	HRESULT preprocess(std::tistream& source, std::tostream& out, const Option& option);
};

}
