using NSubstitute;
using NUnit.Framework;
using System.Diagnostics;
using System.Threading;
using tsm_NET.Generic;

namespace StateMachine.NET.UnitTest.Generic
{
    public class Context : Context<Event, State>
    {
    }

    public class Event : Event<Context>
    {
        public static Event Null = null;
    }

    public class State : State<Context, Event, State>
    {
        public static State Null = null;
    }

    [TestFixture]
    public class Class1
    {
        [Test]
        public void test1()
        {
            var mockEvent = Substitute.For<Event>();
            var mockInitialState = Substitute.For<State>();
            var mockNextState = Substitute.For<State>();

            var c = new Context();
            Assert.That(c.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            // mockInitialState handles mockEvent and changes state to nextState.
            mockInitialState
                .handleEvent(Arg.Is(c), Arg.Is(mockEvent), ref Arg.Is(State.Null))
                .Returns(x =>
                {
                    Trace.WriteLine($"{mockInitialState.GetType()}.handleEvent({x[0]}) is called.");
                    x[2] = mockNextState;
                    return HResult.Ok;
                });

            Assert.That(c.setup(mockInitialState), Is.EqualTo(HResult.Ok));
            Assert.That(c.triggerEvent(mockEvent), Is.EqualTo(HResult.Ok));

            Thread.Sleep(100);

            // Check calls to State objects.
            Received.InOrder(() =>
            {
                mockInitialState.Received()
                    .entry(Arg.Is(c), Arg.Is(Event.Null), Arg.Is(State.Null));
                mockInitialState.Received()
                    .handleEvent(Arg.Is(c), Arg.Is(mockEvent), ref Arg.Is(State.Null));
                mockInitialState.Received()
                    .exit(Arg.Is(c), Arg.Is(mockEvent), mockNextState);
                mockNextState.Received()
                    .entry(Arg.Is(c), Arg.Is(mockEvent), Arg.Is(mockInitialState));
            });
            mockNextState.DidNotReceive().handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());
            mockNextState.DidNotReceive().exit(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>());

            // Current state should be mockNextState.
            Assert.That(c.CurrentState, Is.EqualTo(mockNextState));
        }
    }
}
