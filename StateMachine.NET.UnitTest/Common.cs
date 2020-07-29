using System.Collections;
using System.Runtime.CompilerServices;

namespace Testee
{
    public class Context : tsm_NET.Context
    {
    };

    public class Event : tsm_NET.Event
   {
        public static Event Null { get { return null; } }
    }

    public class State : tsm_NET.State
   {
        public static State Null { get { return null; } }
    }

    public class AsyncContext : tsm_NET.AsyncContext
    {
        public AsyncContext() : base(true) { }
    }

    public class AsyncEvent : Event { };
    public class AsyncState : State { };

namespace Generic
{
    public class Context : tsm_NET.Generic.Context<Event, State>
    {
    };

    public class Event : tsm_NET.Generic.Event<Context>
    {
        public static Event Null { get { return null; } }
    };

    public class State : tsm_NET.Generic.State<Context, Event, State>
    {
        public static State Null { get { return null; } }
    };

    public class AsyncContext : tsm_NET.Generic.AsyncContext<AsyncEvent, AsyncState>
    {
        public AsyncContext() : base(true) { }
    };

    public class AsyncEvent : tsm_NET.Generic.Event<AsyncContext>
    {
        public static AsyncEvent Null { get { return null; } }
    };

    public class AsyncState : tsm_NET.Generic.State<AsyncContext, AsyncEvent, AsyncState>
    {
        public static AsyncState Null { get { return null; } }
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