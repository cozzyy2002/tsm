#include "stdafx.h"
#include "JsonParserObject.h"

using namespace json_parser;

CParserContext::CParserContext()
	: column(0)
{
}

HRESULT CParserContext::start(std::tostream& out, const CJsonParser::Option& option, CParserStateBase* initialState)
{
	previousCharacter = '\0';
	this->outStream = &out;
	this->option = &option;

	return Context::setup(initialState);
}

HRESULT CParserContext::stop()
{
	*outStream << std::ends;
	return Context::shutdown();
}

/*
	Writes character to output stream.
*/
void CParserContext::out(TCHAR character)
{
	// Determine whether we can discard character.
	// In sub state(parsing comment, literal or symbol),
	// characters should be out as is.
	auto canDiscard(!getCurrentState()->isSubState());

	unsigned int columnAdd = 1;
	switch(character) {
	case ' ':
	case '\t':
		// Note: If expand tab, tab is replaced with space.
		//       See CParserState::handleEvent().
		if(canDiscard && option->removeSpace) {
			return;
		}
		break;
	case '\r':
	case '\n':
		if(option->removeEol) {
			return;
		}
		column = columnAdd = 0;
		break;
	}
	*outStream << character;

	// Count column to expand tab.
	column += columnAdd;
}

/*
	Handles characters exept for comment and literal.

	This state is created as initial state on CParserContext::start().
*/
HRESULT CParserState::handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState)
{
	auto option(context->option);
	auto isOut(true);
	switch(e->character) {
	case '*':
		if(option->removeComment) {
			if(context->previousCharacter == '/') {
				// "/*": Start of comment.
				*nextState = new CCommentState(this);
				isOut = false;
			}
		}
		break;
	case '/':
		if(option->removeComment) {
			if(context->previousCharacter == '/') {
				// "//": Start of single line comment.
				*nextState = new CSingleLineCommentState(this);
			}
			isOut = false;
		}
		break;
	case '\"':
		// Start of literal string.
		*nextState = new CLiteralState(this);
		break;
	case '\t':
		// Note: Expand tab affects only outside of literal.
		if(option->expandTab) {
			for(auto c = option->tabStop - (context->column % option->tabStop);
				0 < c; c--) {
				context->out(' ');
			}
			isOut = false;
		}
		break;
	default:
		break;
	}
	if(isOut) {
		if((context->previousCharacter == '/') && option->removeComment) {
			// In case previous '/' is not start of comment.
			context->out('/');
		}
		context->out(e->character);
	}
	return S_OK;
}

/*
	Handles characters in comment.

	Recognaizes '*' followed by '/' as end of the comment.
	Returns back to parent state on end of the comment.
	Other characters than end of line(EOL) are ignored.
*/
HRESULT CCommentState::handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState)
{
	switch(e->character) {
	case '/':
		if(context->previousCharacter == '*') {
			// End of comment
			*nextState = getMasterState();

			// Avoid to out '/' when processing next caracter in CPerserState::handleEvent().
			e->character = '\0';
		}
		break;
	case '\r':
	case '\n':
		// EOL characters.
		context->out(e->character);
		break;
	default:
		break;
	}
	return S_OK;
}

/*
	Handles characters in single line comment.

	Recognaizes end of line(EOL) as end of the comment.
	Other characters than above are ignored.
	Returns back to parent state on end of the comment.
*/
HRESULT CSingleLineCommentState::handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState)
{
	switch(e->character) {
	case '\r':
	case '\n':
		// End of comment
		*nextState = getMasterState();
		context->out(e->character);
		break;
	}
	return S_OK;
}

/*
	Handles characters in literal string.

	Recognizes escape sequence '\x' and end of literal '"'.
	Returns back to parent state on end of literal.
*/
HRESULT CLiteralState::handleEvent(CParserContext* context, CParserEvent* e, tsm::IState** nextState)
{
	static const TCHAR escapeChar('\\');
	auto character(e->character);
	if(context->previousCharacter == escapeChar) {
		// Character following '\' is treated as ordinary character.
		// e.g. '\"' is '\"' not end of literal.
		context->out(escapeChar);
	} else {
		if(character == escapeChar) {
			// On start of escape, do not out now.
			return S_OK;
		} else if(character == '\"') {
			// End of literal string.
			*nextState = getMasterState();
		}
	}
	context->out(character);
	return S_OK;
}
