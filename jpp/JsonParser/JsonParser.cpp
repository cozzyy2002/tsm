#include "stdafx.h"
#include "JsonParser.h"
#include "JsonParserObject.h"

#include <sstream>

using namespace json_parser;

CJsonParser::CJsonParser()
{
}


CJsonParser::~CJsonParser()
{
}

HRESULT CJsonParser::removeComment(LPCTSTR source, bool preserveEol, std::tstring & out)
{
	Option option(true, !preserveEol);
	return preprocess(source, out, option);
}

HRESULT json_parser::CJsonParser::preprocess(LPCTSTR source, std::tstring & out, const Option& option)
{
	std::tistringstream _source(source);
	std::tostringstream _out;

	HRESULT hr = preprocess(_source, _out, option);
	if(SUCCEEDED(hr)) out = _out.str();
	return hr;
}

HRESULT json_parser::CJsonParser::preprocess(std::tistream& source, std::tostream & out, const Option& option)
{
	// Remove space and expand tab can not be specified with each other.
	if(option.removeSpace && option.expandTab) return E_INVALIDARG;
	// Tab stop should be greater than 1.
	if(option.expandTab && (option.tabStop < 2)) return E_INVALIDARG;

	CParserContext context;
	auto stateMachine(context.getStateMachine());
	stateMachine->setLoggerName((stateMachine->getLoggerName() + _T(".JsonParser")).c_str());

	context.start(out, option, new CParserState());

	// Prevent input stream from skipping white space and end of line characters.
	source.unsetf(std::ios_base::skipws);

	TCHAR ch;
	for(source >> ch; !source.eof(); source >> ch) {
		CParserEvent e(ch);
		context.handleEvent(e);
		// Note: e.character might be modified by state.
		context.previousCharacter = e.character;
	}
	context.stop();
	return S_OK;
}
