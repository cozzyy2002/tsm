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
            var mockState = Substitute.For<State>();

            var c = new Context();
            Assert.That(c.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            mockState
                .entry(Arg.Is<Context>(c), Arg.Is<Event>(x => x == null), Arg.Is<State>(x => x == null))
                .Returns(x =>
                {
                    Trace.WriteLine($"{mockState.GetType()}.entry({x[0]}) is called.");
                    return tsm_NET.HResult.Ok;
                });

            Assert.That(c.setup(mockState), Is.EqualTo(HResult.Ok));
            Assert.That(c.CurrentState, Is.EqualTo(mockState), "Context has argument of setup() as initial state.");

            var e = Substitute.For<Event>();
            Assert.That(c.triggerEvent(e), Is.EqualTo(HResult.Ok));

            Thread.Sleep(1000);

            Received.InOrder(() =>
            {
                mockState.Received()
                    .entry(Arg.Is(c), Arg.Is(Event.Null), Arg.Is(State.Null));
                mockState.Received()
                    .handleEvent(Arg.Is(c), Arg.Is(e), ref Arg.Is(State.Null));
            });
        }
    }
}
