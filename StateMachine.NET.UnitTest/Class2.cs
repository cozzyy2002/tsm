using NSubstitute;
using NUnit.Framework;
using System.Diagnostics;
using System.Threading;
using tsm_NET;

namespace StateMachine.NET.UnitTest
{
    [TestFixture]
    public class Class1
    {
        [Test]
        public void test1()
        {
            var mockEvent = Substitute.For<Event>();
            var mockInitialState = Substitute.For<State>();
            var mockNextState = Substitute.For<State>();
            var mockStateMonitor = Substitute.For<IStateMonitor>();

            var c = new Context();
            c.StateMonitor = mockStateMonitor;
            Assert.That(c.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            mockInitialState
                .handleEvent(Arg.Is(c), Arg.Is(mockEvent), ref Arg.Is((State)null))
                .Returns(x =>
                {
                    Trace.WriteLine($"{mockInitialState.GetType()}.handleEvent({x[0]}) is called.");
                    x[2] = mockNextState;
                    return HResult.Ok;
                });

            Assert.That(c.setup(mockInitialState), Is.EqualTo(HResult.Ok));
            Assert.That(c.triggerEvent(mockEvent), Is.EqualTo(HResult.Ok));

            Thread.Sleep(100);

            // Check calls to methods of State.
            Received.InOrder(() =>
            {
                mockInitialState.Received()
                    .entry(Arg.Is(c), Arg.Is((Event)null), Arg.Is((State)null));
                mockInitialState.Received()
                    .handleEvent(Arg.Is(c), Arg.Is(mockEvent), ref Arg.Is((State)null));
                mockInitialState.Received()
                    .exit(Arg.Is(c), Arg.Is(mockEvent), mockNextState);
                mockNextState.Received()
                    .entry(Arg.Is(c), Arg.Is(mockEvent), Arg.Is(mockInitialState));
            });
            mockNextState.DidNotReceive().handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());
            mockNextState.DidNotReceive().exit(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>());

            // Check calls to methods of IStateMonitor.
            mockStateMonitor.Received()
                .onEventTriggered(Arg.Is(c), Arg.Is(mockEvent));
            mockStateMonitor.Received()
                .onEventHandling(Arg.Is(c), Arg.Is(mockEvent), Arg.Is(mockInitialState));
            mockStateMonitor.Received()
                .onStateChanged(Arg.Is(c), Arg.Is(mockEvent), Arg.Is(mockInitialState), Arg.Is(mockNextState));
            mockStateMonitor.Received()
                .onIdle(Arg.Is(c));

            Assert.That(c.CurrentState, Is.EqualTo(mockNextState));
        }
    }
}
