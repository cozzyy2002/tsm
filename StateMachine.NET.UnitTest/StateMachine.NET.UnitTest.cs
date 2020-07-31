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
    [TestFixture(typeof(Testee.Context), typeof(Testee.Event), typeof(Testee.State))]
    [TestFixture(typeof(Testee.AsyncContext), typeof(Testee.AsyncEvent), typeof(Testee.AsyncState))]
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

        // Event::preHandle() returns value except for HResult.Ok(State::handleEvent() is not called).
        [Test]
        public void PreHandleReturnValue([Values(HResult.False, HResult.Abort, HResult.UnExpected)] HResult hr)
        {
            mockEvent.preHandle(context).Returns(hr);

            Assert.That(context.handleEvent(mockEvent), Is.EqualTo(hr));

            mockEvent.DidNotReceiveWithAnyArgs().postHandle(default, default);
            mockState0.DidNotReceiveWithAnyArgs().handleEvent(default, default, ref Arg.Any<TState>());
            mockState0.DidNotReceiveWithAnyArgs().exit(default, default, default);

            Assert.That(context.CurrentState, Is.EqualTo(mockState0));
        }

        // State transition occurs.
        [Test]
        public void StateTransition()
        {
            mockState0.handleEvent(context, mockEvent, ref Arg.Any<TState>())
                .Returns(x => {
                    x[2] = mockState1;
                    return HResult.Ok;
                });

            Assert.That(context.handleEvent(mockEvent), Is.EqualTo(HResult.Ok));

            Received.InOrder(() => {
                mockEvent.Received().preHandle(context);
                mockState0.Received().handleEvent(context, mockEvent, ref Arg.Any<TState>());
                mockState0.Received().exit(context, mockEvent, mockState1);
                mockState1.Received().entry(context, mockEvent, mockState0);
                mockEvent.Received().postHandle(context, HResult.Ok);
            });

            Assert.That(context.CurrentState, Is.EqualTo(mockState1));
        }

        // State::handleEvent() returns error.
        [Test]
        public void HandleEventError()
        {
            var hr = HResult.Abort;

            mockState0.handleEvent(context, mockEvent, ref Arg.Any<TState>())
                .Returns(hr);
            mockEvent.postHandle(context, hr)
                .Returns(hr);

            Assert.That(context.handleEvent(mockEvent), Is.EqualTo(hr));

            Received.InOrder(() => {
                mockEvent.Received().preHandle(context);
                mockState0.Received().handleEvent(context, mockEvent, ref Arg.Any<TState>());
                mockEvent.Received().postHandle(context, hr);
            });
            mockState0.DidNotReceiveWithAnyArgs().exit(default, default, default);

            Assert.That(context.CurrentState, Is.EqualTo(mockState0));
        }

        // State::exit() returns error.
        [Test]
        public void ExitError()
        {
            var hr = HResult.Abort;

            mockState0.handleEvent(context, mockEvent, ref Arg.Any<TState>())
                .Returns(x => {
                    x[2] = mockState1;
                    return HResult.Ok;
                });
            mockState0.exit(context, mockEvent, mockState1)
                .Returns(hr);
            mockEvent.postHandle(context, hr)
                .Returns(hr);

            Assert.That(context.handleEvent(mockEvent), Is.EqualTo(hr));

            Received.InOrder(() => {
                mockEvent.Received().preHandle(context);
                mockState0.Received().handleEvent(context, mockEvent, ref Arg.Any<TState>());
                mockState0.Received().exit(context, mockEvent, mockState1);
                mockEvent.Received().postHandle(context, hr);
            });
            mockState1.DidNotReceiveWithAnyArgs().entry(default, default, default);

            Assert.That(context.CurrentState, Is.EqualTo(mockState0));
        }

        // State::entry() returns error.
        [Test]
        public void EntryError()
        {
            var hr = HResult.Abort;

            mockState0.handleEvent(context, mockEvent, ref Arg.Any<TState>())
                .Returns(x => {
                    x[2] = mockState1;
                    return HResult.Ok;
                });
            mockState1.entry(context, mockEvent, mockState0)
                .Returns(hr);
            mockEvent.postHandle(context, hr)
                .Returns(hr);

            Assert.That(context.handleEvent(mockEvent), Is.EqualTo(hr));

            Received.InOrder(() => {
                mockEvent.Received().preHandle(context);
                mockState0.Received().handleEvent(context, mockEvent, ref Arg.Any<TState>());
                mockState0.Received().exit(context, mockEvent, mockState1);
                mockState1.Received().entry(context, mockEvent, mockState0);
                mockEvent.Received().postHandle(context, hr);
            });

            Assert.That(context.CurrentState, Is.EqualTo(mockState1));
        }

        // State chain: State0 -> State1
        // State1 returns State0 as next state.
        [Test]
        public void SubState_0()
        {
            var mockEvent0 = Substitute.For<TEvent>();
            var mockEvent1 = Substitute.For<TEvent>();

            mockState1 = Substitute.For<TState>(mockState0);
            Assert.That(mockState1.IsSubState, Is.True);
            Assert.That(mockState1.MasterState, Is.EqualTo(mockState0));

            mockState0.handleEvent(context, mockEvent0, ref Arg.Any<TState>())
                .Returns(x => {
                    x[2] = mockState1;
                    return HResult.Ok;
                });
            mockState1.handleEvent(context, mockEvent1, ref Arg.Any<TState>())
                .Returns(x => {
                    x[2] = mockState0;
                    return HResult.Ok;
                });

            Assert.That(context.handleEvent(mockEvent0), Is.EqualTo(HResult.Ok));    // State0 -> State1
            Assert.That(context.handleEvent(mockEvent1), Is.EqualTo(HResult.Ok));    // State1 -> State0(Sub state goes back to master state)

            Received.InOrder(() => {
                // State0 -> State1
                mockEvent0.Received().preHandle(context);
                mockState0.Received().handleEvent(context, mockEvent0, ref Arg.Any<TState>());
                mockState1.Received().entry(context, mockEvent0, mockState0);
                mockEvent0.Received().postHandle(context, HResult.Ok);

                // State1 -> State0(Sub state goes back to master state)
                mockEvent1.Received().preHandle(context);
                mockState1.Received().handleEvent(context, mockEvent1, ref Arg.Any<TState>());
                mockState1.Received().exit(context, mockEvent1, mockState0);
                mockEvent1.Received().postHandle(context, HResult.Ok);
            });

            Assert.That(context.CurrentState, Is.EqualTo(mockState0));
        }
    }
}
