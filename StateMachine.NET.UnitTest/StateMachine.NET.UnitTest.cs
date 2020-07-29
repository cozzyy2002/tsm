// Constant to switch namespace to be tested, tsm_NET or tsm_NET.Generic.
// Both namespaces are not tested together.
//   - tsm_NET.HResult and tsm_NET.Generic.HResult conflict with each other.
//   - `ref Arg.Is<TState>` occurs compile error. See test NoStateTransition().
#define GENERIC_TEST

using NSubstitute;
using NUnit.Framework;
using NUnit.Framework.Constraints;
using System;
using Utils;

#if GENERIC_TEST
using HResult = tsm_NET.Generic.HResult;
#else
using HResult = tsm_NET.HResult;
#endif

namespace StateMachine.NET.UnitTest
{
#if GENERIC_TEST
    [TestFixture(typeof(Testee.Generic.Context), typeof(Testee.Generic.Event), typeof(Testee.Generic.State))]
    [TestFixture(typeof(Testee.Generic.AsyncContext), typeof(Testee.Generic.AsyncEvent), typeof(Testee.Generic.AsyncState))]
#else
    [TestFixture(typeof(Testee.AsyncContext), typeof(Testee.AsyncEvent), typeof(Testee.AsyncState))]
    [TestFixture(typeof(Testee.Context), typeof(Testee.Event), typeof(Testee.State))]
#endif
    class StateMachineUnitTest<TContext, TEvent, TState>
#if GENERIC_TEST
        where TContext : tsm_NET.Generic.Context<TEvent, TState>, new()
        where TEvent : tsm_NET.Generic.Event<TContext>
        where TState : tsm_NET.Generic.State<TContext, TEvent, TState>
#else
        where TContext : tsm_NET.Context, new()
        where TEvent : tsm_NET.Event
        where TState : tsm_NET.State
#endif
    {
        protected TContext context;

        [SetUp]
        public void SetUp()
        {
            context = new TContext();
            Console.WriteLine($"Test start: Context={typeof(TContext)}, IsAsync={context.IsAsync}");
        }

        [TearDown]
        public void TearDown()
        {
            Assert.That(context.CurrentState, Is.Null);
        }
    }

    class StateMachineSetupUnitTest<TContext, TEvent, TState> : StateMachineUnitTest<TContext, TEvent, TState>
#if GENERIC_TEST
        where TContext : tsm_NET.Generic.Context<TEvent, TState>, new()
        where TEvent : tsm_NET.Generic.Event<TContext>
        where TState : tsm_NET.Generic.State<TContext, TEvent, TState>
#else
        where TContext : tsm_NET.Context, new()
        where TEvent : tsm_NET.Event
        where TState : tsm_NET.State
#endif
    {
        // StateMachine.setup(Event = null)
        [Test]
        public void SetupUnitTest_0()
        {
            var mockState = Substitute.For<TState>();

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
            var mockEvent = Substitute.For<TEvent>();
            var mockState = Substitute.For<TState>();

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
            var mockState = Substitute.For<TState>();

            var hr = HResult.UnExpected;
            mockState
                .entry(context, null, null)
                .Returns(hr);

            HResult hrExitCode;
            if (context.IsAsync)
            {
                // AsyncContext::waitRady() should return the error code from State::entry().
                Assert.That(context.setup(mockState), Is.EqualTo(HResult.Ok));
                Assume.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.False));
                Assert.That(context.getAsyncExitCode(out hrExitCode), Is.EqualTo(HResult.Ok));
                Assert.That(hrExitCode, Is.EqualTo(hr));
            }
            else
            {
                // Context::setup() should return the error code from State::entry().
                Assert.That(context.setup(mockState), Is.EqualTo(hr));
                Assert.That(context.getAsyncExitCode(out hrExitCode), Is.EqualTo(HResult.NotImpl));
            }

            Assert.That(context.CurrentState, Is.EqualTo(mockState));
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        // StateMachine::setup() was called twice.
        // 2nd call should fail.
        [Test]
        public void SetupUnitTest_3()
        {
            var mockState = Substitute.For<TState>();

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
            var mockEvent = Substitute.For<TEvent>();
            Assume.That(context.handleEvent(mockEvent), Is.EqualTo(HResult.IllegalMethodCall));
        }
    }

    class StateMachineEventUnitTest<TContext, TEvent, TState> : StateMachineUnitTest<TContext, TEvent, TState>
#if GENERIC_TEST
        where TContext : tsm_NET.Generic.Context<TEvent, TState>, new()
        where TEvent : tsm_NET.Generic.Event<TContext>
        where TState : tsm_NET.Generic.State<TContext, TEvent, TState>
#else
        where TContext : tsm_NET.Context, new()
        where TEvent : tsm_NET.Event
        where TState : tsm_NET.State
#endif
    {
        TState mockState0, mockState1;
        TEvent mockEvent;

        [SetUp]
        public new void SetUp()
        {
            mockState0 = Substitute.For<TState>();
            mockState1 = Substitute.For<TState>();
            mockEvent = Substitute.For<TEvent>();

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

            Received.InOrder(() =>
            {
                mockEvent.Received().preHandle(context);
                mockState0.Received().handleEvent(context, mockEvent,
#if GENERIC_TEST
                    ref Arg.Any<TState>());
#else
                    // When TState = tsm_NET.State, `ref Arg.Any<TState>()` occurs compile error.
                    ref Arg.Any<tsm_NET.State>());
#endif
                mockEvent.Received().postHandle(context, HResult.Ok);
            });
            mockState0.DidNotReceive().exit(Arg.Any<TContext>(), Arg.Any<TEvent>(), Arg.Any<TState>());

            Assert.That(context.CurrentState, Is.EqualTo(mockState0));
        }
    }
}
