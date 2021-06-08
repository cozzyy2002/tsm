using System.Collections;
using System.Runtime.CompilerServices;

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

        public int Id = 0;
    };

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