namespace StateMachine.NET.UnitTest.Common
{
    namespace Async
    {
        public class Context : tsm_NET.Generic.AsyncContext<Event, State>
        {
            public Context() : base(true) { }
        }

        public class Event : tsm_NET.Generic.Event<Context>
        {
            public static Event Null { get { return null; } }
        }

        public class State : tsm_NET.Generic.State<Context, Event, State>
        {
            public static State Null { get { return null; } }
        }
    }
}
