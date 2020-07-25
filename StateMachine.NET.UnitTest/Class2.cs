﻿using NSubstitute;
using NUnit.Framework;
using System;
using System.Threading;
using tsm_NET;

namespace StateMachine.NET.UnitTest
{
    [TestFixture]
    public class AsyncTestCase
    {
        [Test]
        public void BasicTest()
        {
            var mockEvent = Substitute.For<Event>();
            var mockInitialState = Substitute.For<State>();
            var mockNextState = Substitute.For<State>();
            var mockStateMonitor = Substitute.For<IStateMonitor>();

            // StateMachine should run on managed thread to test on NUnit. 
            var c = new AsyncContext(true);
            c.StateMonitor = mockStateMonitor;
            Assert.That(c.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            // mockInitialState handles mockEvent and changes state to nextState.
            mockInitialState
                .handleEvent(c, mockEvent, ref Arg.Is((State)null))
                .Returns(x =>
                {
                    Console.WriteLine($"{mockInitialState.GetType()}.handleEvent({x[0]}) is called.");
                    x[2] = mockNextState;
                    return HResult.Ok;
                });

            Assert.That(c.setup(mockInitialState), Is.EqualTo(HResult.Ok));
            Assert.That(c.triggerEvent(mockEvent), Is.EqualTo(HResult.Ok));
            Thread.Sleep(100);

            // Current state should be mockNextState.
            Assume.That(c.CurrentState, Is.EqualTo(mockNextState));

            // Exit code of worker thread is not retrieved yet.
            HResult hrExitCode;
            Assume.That(c.getAsyncExitCode(out hrExitCode), Is.EqualTo(HResult.IllegalMethodCall));

            // Shutdown
            mockNextState.IsExitCalledOnShutdown = true;
            Assume.That(c.shutdown(), Is.EqualTo(HResult.Ok));
            Thread.Sleep(100);

            // Check exit code of worker thread.
            Assume.That(c.getAsyncExitCode(out hrExitCode), Is.EqualTo(HResult.Ok));
            Assume.That(hrExitCode, Is.EqualTo(HResult.Ok));

            // Check calls to methods of State.
            Received.InOrder(() =>
            {
                mockInitialState.Received()
                    .entry(c, null, null);
                mockInitialState.Received()
                    .handleEvent(c, mockEvent, ref Arg.Is((State)null));
                mockInitialState.Received()
                    .exit(c, mockEvent, mockNextState);
                mockNextState.Received()
                    .entry(c, mockEvent, mockInitialState);
                mockNextState.Received()
                    .exit(c, null, null);
            });
            mockNextState.DidNotReceive().handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());

            // Check calls to methods of IStateMonitor.
            Received.InOrder(() =>
            {
                mockStateMonitor.Received()
                    .onEventHandling(c, mockEvent, mockInitialState);
                mockStateMonitor.Received()
                    .onStateChanged(c, mockEvent, mockInitialState, mockNextState);
                mockStateMonitor.Received()
                    .onIdle(c);
                mockStateMonitor.Received()
                    .onWorkerThreadExit(c, HResult.Ok);
            });
            // onStateChanged() caused by Context.setup() might be called before or after onEventTriggerd().
            mockStateMonitor.Received()
                .onStateChanged(c, null, null, mockInitialState);
            // onEventTriggered() might be called before or after onEventHandling().
            mockStateMonitor.Received()
                .onEventTriggered(c, mockEvent);
        }
    }
}
