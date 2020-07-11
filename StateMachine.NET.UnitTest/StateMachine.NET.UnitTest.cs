using NSubstitute;
using NUnit.Framework;
using StateMachine.NET.UnitTest.Common.Async;
using System;
using tsm_NET.Generic;

namespace StateMachine.NET.UnitTest.Generic
{
    [TestFixture]
    class StateMachineSetupUnitTest
    {
        Context context;
        State state;

        [SetUp]
        public void SetUp()
        {
            context = new Context();
        }

        [TearDown]
        public void TearDown()
        {
            Assert.That(context.CurrentState, Is.EqualTo(State.Null));
        }

        // StateMachine.setup(Event = null)
        [Test]
        public void SetupUnitTest_0()
        {
            var mockState = Substitute.For<State>();

            Assert.That(context.setup(mockState), Is.EqualTo(HResult.Ok));
            Assert.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));

            mockState.Received()
                .entry(Arg.Is(context), Arg.Is(Event.Null), Arg.Is(State.Null));

            Assume.That(context.CurrentState, Is.EqualTo(mockState));
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        // StateMachine.setup(Event = event)
        [Test]
        public void SetupUnitTest_1()
        {
            var mockEvent = Substitute.For<Event>();
            var mockState = Substitute.For<State>();

            Assert.That(context.setup(mockState, mockEvent), Is.EqualTo(HResult.Ok));
            Assert.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));

            mockState.Received()
                .entry(Arg.Is(context), Arg.Is(mockEvent), Arg.Is(State.Null));

            Assume.That(context.CurrentState, Is.EqualTo(mockState));
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }
    }
}
