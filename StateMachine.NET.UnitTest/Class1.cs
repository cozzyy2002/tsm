using NSubstitute;
using NUnit.Framework;
using System;
using System.Diagnostics;
using System.Threading;

namespace StateMachine.NET.UnitTest.Generic
{
    using HResult = tsm_NET.HResult;

    public class Context : tsm_NET.Generic.Context<Event, State>
    {
    }

    public class Event : tsm_NET.Generic.Event<Context>
    {
        public static Event Null = null;
    }

    public class State : tsm_NET.Generic.State<Context, Event, State>
    {
        public override HResult handleEvent(Context context, Event @event, ref State nextState)
        {
            Console.WriteLine($"{this.GetType().ToString()}.handleEvent() is called.");
            Assert.That(context, Is.TypeOf<Context>());
            return HResult.Ok;
        }

        public static State Null = null;
    }

    [TestFixture]
    public class Class1
    {
        [TearDown]
        public void TearDown()
        {
            // Wait for worker thread of StateMachine to process events and to terminate.
            //Thread.Sleep(1000);
        }

        [Test]
        public void test1()
        {
            var mockInitialState = Substitute.For<State>();
            var mockNextState = Substitute.For<State>();

            var c = new Context();
            var e = Substitute.For<Event>();
            Assert.That(c.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            mockInitialState
                .handleEvent(Arg.Is<Context>(c), Arg.Is(e), ref Arg.Any<State>())
                .Returns(x =>
                {
                    Trace.WriteLine($"{mockInitialState.GetType()}.handleEvent({x[0]}) is called.");
                    x[2] = mockNextState;
                    return tsm_NET.HResult.Ok;
                });

            Assert.That(c.setup(mockInitialState), Is.EqualTo(HResult.Ok));
            Assert.That(c.triggerEvent(e), Is.EqualTo(HResult.Ok));

            Thread.Sleep(100);

            Received.InOrder(() =>
            {
                mockInitialState.Received()
                    .entry(Arg.Is(c), Arg.Is(Event.Null), Arg.Is(State.Null));
                mockInitialState.Received()
                    .handleEvent(Arg.Is(c), Arg.Is(e), ref Arg.Is(State.Null));
                mockInitialState.Received()
                    .exit(Arg.Is(c), Arg.Is(e), mockNextState);
                mockNextState.Received()
                    .entry(Arg.Is(c), Arg.Is(e), Arg.Is(mockInitialState));
            });
            mockNextState.DidNotReceive().handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());
            mockNextState.DidNotReceive().exit(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>());

            Assert.That(c.CurrentState, Is.EqualTo(mockNextState));
        }
    }
}
