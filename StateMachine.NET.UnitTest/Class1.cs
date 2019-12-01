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
        public static Event Null { get; } = null;
    }

    public class State : State<Context, Event, State>
    {
        public static State Null { get; } = null;
    }

    [TestFixture]
    public class GenericTestCase
    {
        [Test]
        public void BasicTest()
        {
            var mockEvent = Substitute.For<Event>();
            var mockInitialState = Substitute.For<State>();
            var mockNextState = Substitute.For<State>();
            var mockStateMonitor = Substitute.For<IStateMonitor<Event, State>>();

            var c = new Context();
            c.StateMonitor = mockStateMonitor;
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

            // Current state should be mockNextState.
            Assume.That(c.CurrentState, Is.EqualTo(mockNextState));

            // Shutdown
            mockNextState.CallExitOnShutdown = true;
            Assume.That(c.shutdown(), Is.EqualTo(HResult.Ok));
            Thread.Sleep(100);

            // Check calls to methods of State.
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
                mockNextState.Received()
                    .exit(Arg.Is(c), Arg.Is(Event.Null), Arg.Is(State.Null));
            });
            mockNextState.DidNotReceive().handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());

            // Check calls to methods of IStateMonitor.
            Received.InOrder(() =>
            {
                mockStateMonitor.Received()
                    .onEventTriggered(Arg.Is(c), Arg.Is(mockEvent));
                mockStateMonitor.Received()
                    .onEventHandling(Arg.Is(c), Arg.Is(mockEvent), Arg.Is(mockInitialState));
                mockStateMonitor.Received()
                    .onStateChanged(Arg.Is(c), Arg.Is(mockEvent), Arg.Is(mockInitialState), Arg.Is(mockNextState));
                mockStateMonitor.Received()
                    .onIdle(Arg.Is(c));
                mockStateMonitor.Received()
                    .onWorkerThreadExit(Arg.Is(c), HResult.Ok);
            });
            // onStateChanged() caused by Context.setup() might be called before or after onEventTriggerd().
            mockStateMonitor.Received()
                .onStateChanged(Arg.Is(c), Arg.Is(Event.Null), Arg.Is(State.Null), Arg.Is(mockInitialState));
        }
    }
}
