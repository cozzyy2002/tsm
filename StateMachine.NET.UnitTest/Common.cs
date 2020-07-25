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
            public override string ToString()
            {
                return $"Event#{SequenceNumber}";
            }

            public static Event Null { get { return null; } }
        }

        public class State : tsm_NET.Generic.State<Context, Event, State>
        {
            public override string ToString()
            {
                return $"State#{SequenceNumber}";
            }

            public static State Null { get { return null; } }
        }
    }
}
