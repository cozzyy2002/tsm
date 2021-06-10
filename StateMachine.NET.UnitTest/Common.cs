using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using tsm_NET;

namespace Testee
{
    public class Context : tsm_NET.Context<Event, State>
    {
    };

    public class Event : tsm_NET.Event<Context>
    {
        public Event() : base() { }
        public Event(bool autoDispose) : base(autoDispose) { }
        public Event(int priority) : base(priority) { }
        public Event(int priority, bool autoDispose) : base(priority, autoDispose) { }

        public override string ToString()
        {
            return $"Testee.Event {Id}"; ;
        }
        public int Id = 0;
    };

    public class State : tsm_NET.State<Context, Event, State>
    {
        public State() : base() { }
        public State(bool autoDispose) : base(autoDispose) { }
        public State(State masterState) : base(masterState) { }
        public State(State masterState, bool autoDispose) : base(masterState, autoDispose) { }
    };

    public class AsyncContext : tsm_NET.AsyncContext<AsyncEvent, AsyncState>
    {
        public AsyncContext() : base(true) { }
    };

    public class AsyncEvent : tsm_NET.Event<AsyncContext>
    {
        public AsyncEvent() : base() { }
        public AsyncEvent(bool autoDispose) : base(autoDispose) { }
        public AsyncEvent(int priority) : base(priority) { }
        public AsyncEvent(int priority, bool autoDispose) : base(priority, autoDispose) { }

        public override string ToString()
        {
            return $"Testee.AsyncEvent {Id}"; ;
        }
        public int Id = 0;
    }
    public class AsyncState : tsm_NET.State<AsyncContext, AsyncEvent, AsyncState>
    {
        public AsyncState() : base() { }
        public AsyncState(bool autoDispose) : base(autoDispose) { }
        public AsyncState(AsyncState masterState) : base(masterState) { }
        public AsyncState(AsyncState masterState, bool autoDispose) : base(masterState, autoDispose) { }
    };
}

namespace Utils
{
    public class StateMonitor<C, E, S> : tsm_NET.StateMonitor<C, E, S>
        where C : tsm_NET.IContext
        where E : tsm_NET.IEvent
        where S : tsm_NET.IState
    {
        public override void onIdle(C context)
        {
            addMessage($"onIdle({context})");
        }
        public override void onEventTriggered(C context, E @event)
        {
            addMessage($"onEventTriggered({context}, {@event})");
        }
        public override void onEventHandling(C context, E @event, S current)
        {
            addMessage($"onEventHandling({context}, {@event}, {current}");
        }
        public override void onStateChanged(C context, E @event, S previous, S next)
        {
            addMessage($"onStateChanged({context}, {@event}, {previous}, {next})");
        }
        public override void onTimerStarted(C context, E @event)
        {
            addMessage($"onTimerStarted({context}, {@event})");
        }
        public override void onTimerStopped(C context, E @event, HResult hr)
        {
            addMessage($"onTimerStopped({context}, {@event}, {hr})");
        }
        public override void onWorkerThreadExit(C context, HResult exitCode)
        {
            addMessage($"onWorkerThreadExit({context}, {exitCode})");
        }

        // Returns all messages separated by "\n".
        public string Message { get { return string.Join("\n", Messages); } }

        // Returns massage(s) contain any of words.
        // Each message is separated by "\n".
        public string getMessage(params string[] words)
        {
            return string.Join("\n", Messages.Where(s =>
            {
                foreach(var word in words)
                {
                    if(s.Contains(word)) return true;
                }
                return false;
            }));
        }

        public void addMessage(string str) { Messages.Add($"{DateTime.Now.ToString("ss.fff")} {str}"); }

        public IList<string> Messages = new List<string>();
    }

    public class HResultComparer : IEqualityComparer
    {
        public new bool Equals(object x, object y)
        {
            return (int)x == (int)y;
        }

        public int GetHashCode(object obj)
        {
            return obj.ToString().ToLower().GetHashCode();
        }
    }
}