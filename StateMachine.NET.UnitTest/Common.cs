using System.Collections;
using System.Runtime.CompilerServices;

namespace Testee
{
    public class Context : tsm_NET.Context
    {
    };

    public class Event : tsm_NET.Event
    {
    }

    public class State : tsm_NET.State
    {
        public State() { }
        public State(State masterState) : base(masterState) { }
    }

    public class AsyncContext : tsm_NET.AsyncContext
    {
        public AsyncContext() : base(true) { }
    }

    public class AsyncEvent : Event
    {
    };

    public class AsyncState : tsm_NET.State
    {
        public AsyncState() { }
        public AsyncState(AsyncState masterState) : base(masterState) { }
    };

namespace Generic
{
    public class Context : tsm_NET.Generic.Context<Event, State>
    {
    };

    public class Event : tsm_NET.Generic.Event<Context>
    {
    };

    public class State : tsm_NET.Generic.State<Context, Event, State>
    {
            public State() { }
            public State(State masterState) : base(masterState) { }
    };

    public class AsyncContext : tsm_NET.Generic.AsyncContext<AsyncEvent, AsyncState>
    {
        public AsyncContext() : base(true) { }
    };

    public class AsyncEvent : tsm_NET.Generic.Event<AsyncContext>
    {
    };

    public class AsyncState : tsm_NET.Generic.State<AsyncContext, AsyncEvent, AsyncState>
    {
            public AsyncState() { }
            public AsyncState(AsyncState masterState) : base(masterState) { }
    };
}
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