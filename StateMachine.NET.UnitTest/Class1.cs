﻿using NSubstitute;
using NUnit.Framework;
using System;
using System.Threading;
using tsm_NET;
using Assert = NUnit.Framework.Assert;

namespace StateMachine.NET.UnitTest
{
    [TestFixture]
    public class TestCase
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

        [Test]
        public void BasicTest()
        {
            var mockEvent = Substitute.For<Event>();
            var mockInitialState = Substitute.For<State>();
            var mockNextState = Substitute.For<State>();
            var mockStateMonitor = Substitute.For<StateMonitor<Context, Event, State>>();

            // Create synchronous Context object.
            var c = new Context();
            c.StateMonitor = mockStateMonitor;
            Assert.That(c.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            // mockInitialState handles mockEvent and changes state to nextState.
            mockInitialState
                .handleEvent(c, mockEvent, ref Arg.Is(State.Null))
                .Returns(x =>
                {
                    Console.WriteLine($"{mockInitialState.GetType()}.handleEvent({x[0]}) is called.");
                    x[2] = mockNextState;
                    return HResult.Ok;
                });

            Assert.That(c.setup(mockInitialState), Is.EqualTo(HResult.Ok));
            Assert.That(c.handleEvent(mockEvent), Is.EqualTo(HResult.Ok));

            // Current state should be mockNextState.
            Assume.That(c.CurrentState, Is.EqualTo(mockNextState));

            // Shutdown
            mockNextState.IsExitCalledOnShutdown = true;
            Assume.That(c.shutdown(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));

            // Check calls to methods of State.
            Received.InOrder(() =>
            {
                mockInitialState.Received()
                    .entry(c, null, null);
                mockInitialState.Received()
                    .handleEvent(c, mockEvent, ref Arg.Is(State.Null));
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
                    .onStateChanged(c, null, null, mockInitialState);
                mockStateMonitor.Received()
                    .onEventHandling(c, mockEvent, mockInitialState);
                mockStateMonitor.Received()
                    .onStateChanged(c, mockEvent, mockInitialState, mockNextState);
            });
            mockStateMonitor.DidNotReceive().onEventTriggered(Arg.Any<Context>(), Arg.Any<Event>());
            mockStateMonitor.DidNotReceive().onIdle(Arg.Any<Context>());
            mockStateMonitor.DidNotReceive().onWorkerThreadExit(Arg.Any<Context>(), Arg.Any<HResult>());
        }
    }

    [TestFixture]
    public class AsyncTestCase
    {
        public class Context : AsyncContext<Event, State>
        {
            // StateMachine should run on managed thread to test on NUnit. 
            public Context() : base(true) { }
        }

        public class Event : Event<Context>
        {
            public static Event Null { get; } = null;
        }

        public class State : State<Context, Event, State>
        {
            public static State Null { get; } = null;
        }

        Context context;
        Event mockEvent;
        State mockInitialState;
        State mockNextState;
        StateMonitor<Context, Event, State> mockStateMonitor;

        string Now { get { return DateTime.Now.ToString("ss.fff"); } }

        [SetUp]
        public void SetUp()
        {
            Console.WriteLine($"{Now} SetUp()");
            context = new Context();
            mockEvent = Substitute.For<Event>();
            mockInitialState = Substitute.For<State>();
            mockNextState = Substitute.For<State>();
            mockStateMonitor = Substitute.For<StateMonitor<Context, Event, State>>();
        }

        [TearDown]
        public void TearDown()
        {
            Console.WriteLine($"{Now} TearDown().");
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        [Test]
        public void BasicTest()
        {
            context.StateMonitor = mockStateMonitor;
            Assert.That(context.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            // mockInitialState handles mockEvent and changes state to nextState.
            mockInitialState
                .handleEvent(context, mockEvent, ref Arg.Is(State.Null))
                .Returns(x =>
                {
                    Console.WriteLine($"{mockInitialState.GetType()}.handleEvent({x[0]}) is called.");
                    x[2] = mockNextState;
                    return HResult.Ok;
                });

            Assert.That(context.setup(mockInitialState), Is.EqualTo(HResult.Ok));

            Assert.That(context.triggerEvent(mockEvent), Is.EqualTo(HResult.Ok));
            Thread.Sleep(100);

            // Current state should be mockNextState.
            Assert.That(context.CurrentState, Is.EqualTo(mockNextState));

            // Exit code of worker thread is not retrieved yet.
            HResult hrExitCode;
            Assume.That(context.getAsyncExitCode(out hrExitCode), Is.EqualTo(HResult.IllegalMethodCall));

            // Shutdown
            mockNextState.IsExitCalledOnShutdown = true;
            Assume.That(context.shutdown(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));
            Thread.Sleep(100);

            // Check exit code of worker thread.
            Assume.That(context.getAsyncExitCode(out hrExitCode), Is.EqualTo(HResult.Ok));
            Assume.That(hrExitCode, Is.EqualTo(HResult.Ok));

            // Check calls to methods of State.
            Received.InOrder(() =>
            {
                mockInitialState.Received()
                    .entry(context, null, null);
                mockInitialState.Received()
                    .handleEvent(context, mockEvent, ref Arg.Is(State.Null));
                mockInitialState.Received()
                    .exit(context, mockEvent, mockNextState);
                mockNextState.Received()
                    .entry(context, mockEvent, mockInitialState);
                mockNextState.Received()
                    .exit(context, null, null);
            });
            mockNextState.DidNotReceive().handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());

            // Check calls to methods of IStateMonitor.
            Received.InOrder(() =>
            {
                mockStateMonitor.Received()
                    .onEventHandling(context, mockEvent, mockInitialState);
                mockStateMonitor.Received()
                    .onStateChanged(context, mockEvent, mockInitialState, mockNextState);
                mockStateMonitor.Received()
                    .onIdle(context);
                mockStateMonitor.Received()
                    .onWorkerThreadExit(context, HResult.Ok);
            });
            // onStateChanged() caused by Context.setup() might be called before or after onEventTriggerd().
            mockStateMonitor.Received()
                .onStateChanged(context, null, null, mockInitialState);
            // onEventTriggered() might be called before or after onEventHandling().
            mockStateMonitor.Received()
                .onEventTriggered(context, mockEvent);
        }

        [Test]
        public void TimerTest()
        {
            context.StateMonitor = mockStateMonitor;

            mockStateMonitor
                .When(x => x.onEventTriggered(Arg.Any<Context>(), Arg.Any<Event>()))
                .Do(x => { Console.WriteLine($"{Now} onEventTriggered()"); });
            mockStateMonitor
                .When(x => x.onEventHandling(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>()))
                .Do(x => { Console.WriteLine($"{Now} onEventHandling()"); });
            mockStateMonitor
                .When(x => x.onTimerStarted(Arg.Any<Context>(), Arg.Any<Event>()))
                .Do(x => { Console.WriteLine($"{Now} onTimerStarted({x[1]})"); });

            var count = 4;
            mockInitialState
                .handleEvent(context, mockEvent, ref Arg.Is(State.Null))
                .Returns(x =>
                {
                    count--;
                    var e = x[1] as Event;
                    Console.WriteLine($"{Now} handleEvent: count={count}");

                    if (count == 0) {
                        Console.WriteLine("  Changing state to next state.");
                        x[2] = mockNextState;
                    }
                    return HResult.Ok;
                });

            Assert.That(context.setup(mockInitialState), Is.EqualTo(HResult.Ok));
            mockEvent.setTimer(mockInitialState, TimeSpan.FromMilliseconds(200), TimeSpan.FromMilliseconds(100));
            Assume.That(mockEvent.DelayTime, Is.EqualTo(TimeSpan.FromMilliseconds(200)));
            Assume.That(mockEvent.IntervalTime, Is.EqualTo(TimeSpan.FromMilliseconds(100)));

            Console.WriteLine($"{Now} Triggering event: Timer: Delay={mockEvent.DelayTime:fff}, Intervale={mockEvent.IntervalTime:fff}");
            Assert.That(context.triggerEvent(mockEvent), Is.EqualTo(HResult.Ok));

            var pendingEvents = mockInitialState.PendingEvents;
            Console.WriteLine($"{Now} Pending events count={pendingEvents.Count}");
            Assert.That(pendingEvents.Count, Is.EqualTo(1));
            Assume.That(pendingEvents[0], Is.EqualTo(mockEvent));
            Assume.That(context.PendingEvents.Count, Is.EqualTo(0));

            Thread.Sleep(2000);
            Console.WriteLine($"{Now} End.");
            Assert.That(context.CurrentState, Is.EqualTo(mockNextState));
            Assume.That(count, Is.EqualTo(0));

            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));

            mockInitialState.Received()
                .handleEvent(context, mockEvent, ref Arg.Is(State.Null));
        }
    }

    [TestFixture]
    public class AssertTestCase
    {
        class Context : Context<Event, State> { }
        class Event : Event<Context> { }
        class State : State<Context, Event, State> { }

        Context context = new Context();
        Event @event = new Event();

        // Interface to create mock for Assert delegates.
        public interface IAssert
        {
            void proc(HResult hr, string exp, string sourceFile, int line);
            void writer(string msg);
        }

        IAssert assert;

        [SetUp]
        public void SetUp()
        {
            // Reset stattic Assert delegates.
            tsm_NET.Assert.OnAssertFailedProc = null;
            tsm_NET.Assert.OnAssertFailedWriter = null;

            assert = Substitute.For<IAssert>();
        }

        // Test for OnAssertFailedProc.
        [Test]
        public void AssertProcTest()
        {
            tsm_NET.Assert.OnAssertFailedProc = assert.proc;

            assert
                .When(x => x.proc(Arg.Any<HResult>(), Arg.Any<string>(), Arg.Any<string>(), Arg.Any<int>()))
                .Do(x =>
                {
                    Console.WriteLine($"OnAssertFailedProc(HResult=0x{x[0]:x})");
                });
            var hr = context.handleEvent(@event);
            Console.WriteLine($"HResult returned = 0x{hr,08:x}");

            assert.Received()
                .proc(hr, Arg.Any<string>(), Arg.Any<string>(), Arg.Any<int>());
            assert.DidNotReceiveWithAnyArgs()
                .writer(default);
        }

        // Test for OnAssertFailedWriter.
        [Test]
        public void AssertWriterTest()
        {
            var assertMessage = "";
            tsm_NET.Assert.OnAssertFailedWriter = assert.writer;

            assert
                .When(x => x.writer(Arg.Any<string>()))
                .Do(args =>
                {
                    assertMessage = (string)args[0];
                    Console.WriteLine($"Assert.OnAssertFailedWriter(`{assertMessage}`)");
                });

            var hr = context.handleEvent(@event);
            Console.WriteLine($"HResult returned = 0x{hr,08:x}");

            assert.DidNotReceiveWithAnyArgs()
                .proc(default, default, default, default);
            assert.Received()
                .writer(Arg.Any<string>());

            Assert.That(assertMessage.Contains($"{hr:x}"));
        }
    }
}
