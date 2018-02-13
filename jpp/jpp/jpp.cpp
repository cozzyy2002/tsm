// jpp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../JsonParser/JsonParser.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <Shlwapi.h>
#include <locale>

using namespace json_parser;

namespace std {
#ifdef _UNICODE
static auto& tcin(wcin);
static auto& tcout(wcout);
static auto& tcerr(wcerr);
using tifstream = wifstream;
#else
static auto& tcin(cin);
static auto& tcout(cout);
static auto& tcerror(cerr);
using tifstream = ifstream;
#endif
}

static auto usage =
"JSON preprocessor\n"
"Usage: jpp [-csre [N]] [json]\n"
"       -c:      Remove comment(default)\n"
"       -s:      Remove Space and Tab\n"
"       -r:      Remove EOL(CR and LF)\n"
"       -e [N]:  Expand tab for each N columns(default=2)\n"
"       Note: -s and -e are not specified with each other.\n"
"       json: JSON file to preprocess.\n"
"             If `json` is existing file, jpp reads JSON text from the file.\n"
"             Else jpp recognizes `json` as JSON text.\n"
"             If this argument is omitted, jpp reads STDIN.\n"
;

static const int defaultTabStop = 2;
static int checkArgs(int argc, TCHAR* argv[], CJsonParser::Option& option);
static int checkOptions(TCHAR* options, CJsonParser::Option& option);

int _tmain(int argc, TCHAR *argv[])
{
	CJsonParser::Option option;
	auto i = checkArgs(argc, argv, option);
	if(i < 0) return 1;
	if(option.removeSpace && option.expandTab) {
		std::cerr << "Error: Remove space and Expand tab are specified." << std::endl;
		return 1;
	}
	if(!option.removeSpace && !option.removeSpace && !option.removeEol && !option.expandTab) {
		// If no option is specified, set default.
		option.removeComment = true;
	}

	std::tcerr << _T("Option:");
	if(option.removeComment) std::tcerr << _T(" Remove comment");
	if(option.removeSpace) std::tcerr << _T(" Remove space");
	if(option.removeEol) std::tcerr << _T(" Remove EOL");
	if(option.expandTab) std::tcerr << _T(" Expand tab: ") << option.tabStop;
	std::tcerr << std::endl;

	std::tistream* in = nullptr;
	if(i < argc) {
		auto lastArg = argv[i];
		if(PathFileExists(lastArg) && !PathIsDirectory(lastArg)) {
			// Existing file
			std::tcerr << _T("Read JSON from file: ") << lastArg << std::endl;
			static std::tifstream istream(lastArg);
			in = &istream;
		} else {
			// JSON text in command line.
			std::tcerr << _T("JSON text: ") << lastArg << std::endl;
			static std::tistringstream istream(lastArg);
			in = &istream;
		}
	} else {
		// Read JSON from STDIN.
		std::tcerr << _T("Read JSON from STDIN.") << std::endl;
		in = &std::tcin;
	}

	CJsonParser parser;
	parser.preprocess(*in, std::tcout, option);
	return 0;
}

enum {
	CHECK_OPTIONS_OK,
	CHECK_OPTIONS_TAB_STOP,
	CHECK_OPTIONS_ERROR,
};

/*static*/ int checkArgs(int argc, TCHAR* argv[], CJsonParser::Option& option)
{
	int i;
	for(i = 1; i < argc; i++) {
		if(*argv[i] == _T('-')) {
			switch(checkOptions(argv[i] + 1, option)) {
			case CHECK_OPTIONS_OK:
				break;
			case CHECK_OPTIONS_TAB_STOP:
				if((i + 1) < argc) {
					auto str = argv[i + 1];
					std::locale loc;
					if(isdigit(*str, loc)) {
						// Next argument should be legal number as tab stop.
						auto tabStop = _tstoi(str);
						if((1 < tabStop) && (tabStop <= 8) && (errno == 0)) {
							option.tabStop = (unsigned int)tabStop;
							i++;
						} else {
							std::tcerr << _T("Error: Illegal tab stop: ") << str << std::endl;
							return -1;
						}
					} else {
						option.tabStop = defaultTabStop;
					}
				} else {
					option.tabStop = defaultTabStop;
				}
				break;
			case CHECK_OPTIONS_ERROR:
			default:
				return -1;
			}
		} else {
			break;
		}
	}
	return i;
}

/*static*/ int checkOptions(TCHAR* options, CJsonParser::Option& option)
{
	auto ret(CHECK_OPTIONS_OK);
	for(auto p = options; *p; p++) {
		switch(*p) {
		case 'c': option.removeComment = true; break;
		case 's': option.removeSpace = true; break;
		case 'r': option.removeEol = true; break;
		case 'e': option.expandTab = true; ret = CHECK_OPTIONS_TAB_STOP; break;
		case '?':
		case 'h':
			std::cout << usage;
			return CHECK_OPTIONS_ERROR;
		default:
			std::tcerr << _T("Error: Unknown option: ") << *p << std::endl;
			return CHECK_OPTIONS_ERROR;
		}
	}
	return ret;
}
