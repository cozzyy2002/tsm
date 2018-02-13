#pragma once

#include "JsonParser.h"
#include <iostream>

namespace json_parser {

class CParserContext;
class CParserEvent;
class CParserState;

using CParserStateBase = tsm::State<CParserContext, CParserEvent>;

class CParserContext : public tsm::Context<CParserEvent, CParserStateBase>
{
public:
	CParserContext();

	HRESULT start(std::tostream& out, const CJsonParser::Option& option, CParserStateBase* initialState);
	HRESULT stop();
	void out(TCHAR character);

	const CJsonParser::Option* option;
	TCHAR previousCharacter;
	// Column number used to expand tab.
	unsigned int column;

protected:
	// Output string written by out() method.
	std::tostream* outStream;
};

// State that processes ordinary characters.
// Ordinary characters are other than comment nor literal string.
// This state is also created as initial state.
class CParserState : public CParserStateBase
{
public:
	virtual HRESULT handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState) override;
};

// Comment state started by "/*" and ended by "*/"
class CCommentState : public CParserStateBase
{
public:
	CCommentState(CParserState* masterState) : State(masterState) {}
	virtual HRESULT handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState) override;
};

// Comment state started by "//" and ended by end of line(EOL)
class CSingleLineCommentState : public CParserStateBase
{
public:
	CSingleLineCommentState(CParserState* masterState) : State(masterState) {}
	virtual HRESULT handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState) override;
};

// Literal state enclosed by single/double quotation mark.
class CLiteralState : public CParserStateBase
{
public:
	CLiteralState(CParserState* masterState) : State(masterState) {}
	virtual HRESULT handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState) override;
};

class CParserEvent : public tsm::Event
{
public:
	CParserEvent(TCHAR character) : character(character) {}

	// Character to parse.
	TCHAR character;
};

}
