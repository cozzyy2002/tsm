#pragma once

#include "JsonParser.h"
#include <iostream>

namespace json_parser {

class CParserContext : public state_machine::Context
{
public:
	CParserContext();

	HRESULT start(std::tostream& out, const CJsonParser::Option& option, state_machine::State* initialState);
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
class CParserState : public state_machine::State
{
public:
	virtual HRESULT handleEvent(state_machine::Context& context, state_machine::Event& e, State& currentState, State** nextState) override;
};

// Comment state started by "/*" and ended by "*/"
class CCommentState : public state_machine::State
{
public:
	CCommentState(CParserState* masterState) : State(masterState) {}
	virtual HRESULT handleEvent(state_machine::Context& context, state_machine::Event& e, State& currentState, State** nextState) override;
};

// Comment state started by "//" and ended by end of line(EOL)
class CSingleLineCommentState : public state_machine::State
{
public:
	CSingleLineCommentState(CParserState* masterState) : State(masterState) {}
	virtual HRESULT handleEvent(state_machine::Context& context, state_machine::Event& e, State& currentState, State** nextState) override;
};

// Literal state enclosed by single/double quotation mark.
class CLiteralState : public state_machine::State
{
public:
	CLiteralState(CParserState* masterState) : State(masterState) {}
	virtual HRESULT handleEvent(state_machine::Context& context, state_machine::Event& e, State& currentState, State** nextState) override;
};

class CParserEvent : public state_machine::Event
{
public:
	CParserEvent(TCHAR character) : character(character) {}

	// Character to parse.
	TCHAR character;

	// Suppress log this event.
	virtual LogLevel getLogLevel() const override { return 0; }
};

}
