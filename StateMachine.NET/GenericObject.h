#pragma once

#include "StateMachine.NET.h"

namespace tsm_NET
{
namespace Generic
{

generic<typename E, typename S>
public ref class Context : public tsm_NET::Context
{
public:
	Context(bool isAsync) : tsm_NET::Context(isAsync) {}
	virtual ~Context() {}

	HResult setup(S initialState, E event) { return tsm_NET::Context::setup((tsm_NET::State^)initialState, (tsm_NET::Event^)event); }
	HResult setup(S initialState) { return tsm_NET::Context::setup((tsm_NET::State^)initialState); }
	HResult shutdown(TimeSpan timeout) { return tsm_NET::Context::shutdown(timeout); }
	HResult shutdown() { return tsm_NET::Context::shutdown(); }
	HResult triggerEvent(E event) { return tsm_NET::Context::triggerEvent((tsm_NET::Event^)event); }
	HResult handleEvent(E event) { return tsm_NET::Context::handleEvent((tsm_NET::Event^)event); }
	HResult waitReady(TimeSpan timeout) { return tsm_NET::Context::waitReady(timeout); }
	S getCurrentState() { return (S)tsm_NET::Context::getCurrentState(); }

	property S CurrentState { S get() { return getCurrentState(); }}
};

generic<typename C, typename E, typename S>
public ref class State : public tsm_NET::State
{
public:
	State() : tsm_NET::State(nullptr) {}
	State(S masterState) : tsm_NET::State((tsm_NET::State^)masterState) {}
	~State() {}

#pragma region Methods to be implemented by sub class.
	virtual HResult handleEvent(C context, E event, S% nextState) { return HResult::Ok; }
	virtual HResult entry(C context, E event, S previousState) { return HResult::Ok; }
	virtual HResult exit(C context, E event, S nextState) { return HResult::Ok; }
#pragma endregion

#pragma region Methods that call sub class with generic parameters.
	virtual HResult handleEvent(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^% nextState) override {
		S _nextState;
		auto ret = handleEvent((C)context, (E)event, _nextState);
		if(_nextState) {
			nextState = (tsm_NET::State^)_nextState;
		}
		return ret;
	}
	virtual HResult entry(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ previousState) override {
		return entry((C)context, (E)event, (S)previousState);
	}
	virtual HResult exit(tsm_NET::Context^ context, tsm_NET::Event^ event, tsm_NET::State^ nextState) override {
		return exit((C)context, (E)event, (S)nextState);
	}
#pragma endregion

	S getMasterState() { return (S)tsm_NET::State::getMasterState(); }

	property S MasterState { S get() { return getMasterState(); }}
};

generic<typename C>
public ref class Event : public tsm_NET::Event
{
public:
	~Event() {}

#pragma region Methods to be implemented by sub class.
	virtual HResult preHandle(C context) { return HResult::Ok; }
	virtual HResult postHandle(C context, HResult hr) { return hr; }
#pragma endregion

#pragma region Methods that call sub class with generic parameters.
	virtual HResult preHandle(tsm_NET::Context^ context) override {
		return preHandle((C)context);
	}
	virtual HResult postHandle(tsm_NET::Context^ context, HResult hr) override {
		return postHandle((C)context, hr);
	}
#pragma endregion
};
}
}
