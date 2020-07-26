using NSubstitute;
using NUnit.Framework;
using StateMachine.NET.UnitTest.Common.Async;
using System;
using System.Threading;
using tsm_NET.Generic;

namespace StateMachine.NET.UnitTest.Generic
{
    class StateMachineUnitTest
    {
        protected Context context;

        [SetUp]
        public void SetUp()
        {
            context = new Context();

            // Ensure that AsyncContext has been created.
            Assert.That(context.IsAsync, Is.True);
        }

        [TearDown]
        public void TearDown()
        {
            Assert.That(context.CurrentState, Is.EqualTo(State.Null));
        }
    }

    [TestFixture]
    class StateMachineSetupUnitTest : StateMachineUnitTest
    {
        // StateMachine.setup(Event = null)
        [Test]
        public void SetupUnitTest_0()
        {
            var mockState = Substitute.For<State>();

            Assert.That(context.setup(mockState), Is.EqualTo(HResult.Ok));
            Assert.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));

            mockState.Received()
                .entry(context, null, null);

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
                .entry(context, mockEvent, null);

            Assume.That(context.CurrentState, Is.EqualTo(mockState));
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        // State::entry() returns error
        [Test]
        public void SetupUnitTest_2()
        {
            var mockState = Substitute.For<State>();

            var hr = HResult.UnExpected;
            mockState
                .entry(context, null, null)
                .Returns(hr);

            Assert.That(context.setup(mockState), Is.EqualTo(HResult.Ok));
            Assume.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.False));
            HResult hrExitCode;
            Assert.That(context.getAsyncExitCode(out hrExitCode), Is.EqualTo(HResult.Ok));
            Assert.That(hrExitCode, Is.EqualTo(hr));

            Assert.That(context.CurrentState, Is.EqualTo(mockState));
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        // StateMachine::setup() was called twice.
        // 2nd call should fail.
        [Test]
        public void SetupUnitTest_3()
        {
            var mockState = Substitute.For<State>();

            Assert.That(context.setup(mockState), Is.EqualTo(HResult.Ok));
            Assert.That(context.setup(mockState), Is.EqualTo(HResult.IllegalMethodCall));
            Assert.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));

            mockState.Received()
                .entry(context, null, null);

            Assume.That(context.CurrentState, Is.EqualTo(mockState));
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        // StateMachine::shutdown() is called before setup().
        [Test]
        public void SetupUnitTest_4()
        {
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
            Assume.That(context.CurrentState, Is.EqualTo(null));
        }
        // StateMachine::handleEvent() is called before setup().
        [Test]
        public void SetupUnitTest_5()
        {
            var mockEvent = Substitute.For<Event>();
            Assume.That(context.handleEvent(mockEvent), Is.EqualTo(HResult.IllegalMethodCall));
        }
    }

    [TestFixture]
    class StateMachineEventUnitTest : StateMachineUnitTest
    {
        State mockState0, mockState1;
        Event mockEvent;

        [SetUp]
        public new void SetUp()
        {
            mockState0 = Substitute.For<State>();
            mockState1 = Substitute.For<State>();
            mockEvent = Substitute.For<Event>();

            Assert.That(context.setup(mockState0), Is.EqualTo(HResult.Ok));
            Assert.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));

            mockState0.Received().entry(context, null, null);
        }
        [TearDown]
        public new void TearDown()
        {
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        // No state transition occurs.
        // Event::preHandle() returns S_OK.
        [Test]
        public void NoStateTransition()
        {
            Assert.That(context.handleEvent(mockEvent), Is.EqualTo(HResult.Ok));

            Received.InOrder(() => {
                mockEvent.Received().preHandle(context);
                mockState0.Received().handleEvent(context, mockEvent, ref Arg.Is(State.Null));
                mockEvent.Received().postHandle(context, HResult.Ok);
            });
            mockState0.DidNotReceive().exit(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>());

            Assert.That(context.CurrentState, Is.EqualTo(mockState0));
        }
    }
}
