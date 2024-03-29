﻿using NSubstitute;
using NSubstitute.ReceivedExtensions;
using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Threading;

namespace NET.TimerUnitTest
{
    using Context = Testee.AsyncContext;
    using Event = Testee.AsyncEvent;
    using HResult = tsm_NET.HResult;
    using State = Testee.AsyncState;

    [TestFixture]
    class TimerUnitTest
    {
        [SetUp]
        public void SetUp()
        {
            context = new Context();
            mockState0 = Substitute.For<State>();
            mockState1 = Substitute.For<State>();
            e0 = new Event();
            e1 = new Event();
            mockState0.IsExitCalledOnShutdown = true;
            Assert.That(context.setup(mockState0), Is.EqualTo(HResult.Ok));
            Assert.That(context.waitReady(1000), Is.EqualTo(HResult.Ok));
            mockState0.Received().entry(context, null, null);
        }

        [TearDown]
        public void TearDown()
        {
            Assert.That(context.shutdown(1000), Is.EqualTo(HResult.Ok));
        }

        protected Context context;
        protected Event e0, e1;
        protected State mockState0, mockState1;

        // Cancel event timer of state on State::exit()
        [Test]
        public void CancelStateTimer()
        {
            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
        }

        // Cancel event timer of state on State::exit()
        [Test]
        public void CancelStateTimerByStateChange()
        {
            mockState0.handleEvent(context, e0, ref Arg.Any<State>())
                .Returns(x => {
                    x[2] = mockState1;
                    return HResult.Ok;
            });

            // Start interval timer to be canceled.
            e1.setTimer(mockState0, 50, 100);
            Assert.That(context.triggerEvent(e1), Is.EqualTo(HResult.Ok));

            // Wait for e1 to be handled twice.
            // Then trigger e0 to change state mockState0 to mockState1.
            Thread.Sleep(200);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));

            Thread.Sleep(100);
            Received.InOrder(() => {
                mockState0.handleEvent(context, e1, ref Arg.Any<State>());
                mockState0.handleEvent(context, e1, ref Arg.Any<State>());
                mockState0.exit(context, e0, mockState1);
                mockState1.entry(context, e0, mockState0);
            });

            Assert.That(context.CurrentState, Is.EqualTo(mockState1));
        }
    }

    [TestFixture(true)]     // Use Context as ITimerOwner
    [TestFixture(false)]    // Use State as ITimerOwner
    class TimerClientUnitTest : TimerUnitTest
    {
        public TimerClientUnitTest(bool isTmerOwnerContext)
        {
            this.isTmerOwnerContext = isTmerOwnerContext;
        }

        [SetUp]
        public new void SetUp()
        {
            // Set timerOwner according to argument of TestFixture attribute.
            timerOwner = isTmerOwnerContext ? (tsm_NET.ITimerOwner)context : mockState0;
        }

        bool isTmerOwnerContext;
        tsm_NET.ITimerOwner timerOwner;

        [Test]
        public void CancelTimerReturnValueTest()
        {
            // Timer is not set.
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.IllegalMethodCall));
            e0.setDelayTime(timerOwner, TimeSpan.Zero);
            // Timer is not started.
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.TimerIsStopped));
        }

        // One-shot timer
        [Test]
        public void OneShotTimer()
        {
            Console.WriteLine($"One-shot timer test using {timerOwner}");

            e0.setDelayTime(timerOwner, 100);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerOwner.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);
            Thread.Sleep(100);

            // Timer should be stopped.
            events = timerOwner.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(0));

            // Timer event should have been handled once.
            mockState0.Received()
                .handleEvent(context, e0, ref Arg.Any<State>());
        }

        // Cancel one-shot timer
        [Test]
        public void CancelOneShotTimer()
        {
            Console.WriteLine($"Cancel one-Shot timer test using {timerOwner}");

            e0.setDelayTime(timerOwner, 100);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerOwner.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.Ok));

            // Timer should be canceled.
            events = timerOwner.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(0));
            Thread.Sleep(100);

            // Timer event should have been handled once.
            mockState0.DidNotReceive()
                .handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());
        }

        // Interval timer(Including cancel)
        [Test]
        public void IntervalTimer()
        {
            Console.WriteLine($"Interval timer test using {timerOwner}");

            e0.setTimer(timerOwner, 100, 200);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerOwner.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);
            Thread.Sleep(500);
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.Ok));

            // Timer should be stopped.
            events = timerOwner.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(0));

            // Timer event should have been handled 3 times(Delay x 1 + Interval x 2).
            mockState0.Received(3)
                .handleEvent(context, e0, ref Arg.Any<State>());
        }

        [Test]
        public void CancelbyShutdown()
        {
            e0.setDelayTime(timerOwner, 100);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerOwner.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);

            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
            Thread.Sleep(100);

            if (timerOwner == context)
            {
                // Timer should be stopped.
                events = timerOwner.PendingEvents;
                Assert.That(events.Count, Is.EqualTo(0));
            }
            else
            {
                // NOTE:
                // `timerOwner.PendingEvents` can not be called when timerOwner is State object
                // because State object is disposed by Context.Shutdown().
            }

            // Timer event should have been handled once.
            mockState0.DidNotReceive()
                .handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());
        }

        [Test]
        public void CancelInHandleEvent()
        {
            var count = 3;
            mockState0.handleEvent(context, e0, ref Arg.Any<State>())
                .Returns(x => {
                    Console.WriteLine($" Time: {DateTime.Now:ss.fff}");
                    if(--count == 0) e0.cancelTimer();
                    return HResult.Ok;
                });

            Console.WriteLine($" Time: {DateTime.Now:ss.fff}");
            e0.setTimer(timerOwner, 50, 100);
            Assert.That(context.handleEvent(e0), Is.EqualTo(HResult.Ok));
            Assert.That(timerOwner.PendingEvents.Count, Is.EqualTo(1));
            Thread.Sleep(500);
            Assert.That(timerOwner.PendingEvents.Count, Is.EqualTo(0));
        }

        [Test]
        public void CancelOtherTimerTest()
        {
            mockState0.handleEvent(context, e0, ref Arg.Any<State>())
                .Returns(x => {
                    // Timer thread should terminate at once.
                    Assert.That(e1.cancelTimer(1), Is.EqualTo(HResult.Ok));
                    return HResult.Ok;
                });

            // Event to cancel e1.
            e0.setDelayTime(timerOwner, 50);
            // Event to be canceled.
            e1.setTimer(timerOwner, 100, 50);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Assert.That(context.triggerEvent(e1), Is.EqualTo(HResult.Ok));

            Assert.That(timerOwner.PendingEvents.Count, Is.EqualTo(2));
            Thread.Sleep(200);
            Assert.That(timerOwner.PendingEvents.Count, Is.EqualTo(0));
            mockState0.DidNotReceive()
                .handleEvent(Arg.Any<Context>(), e1, ref Arg.Any<State>());
        }

        [Test]
        public void TimerAccuracyTest()
        {
            var expectedTimes = new int[] { 100, 200, 200, 200, 200 };
            var times = new List<DateTime>();
            mockState0.handleEvent(context, e0, ref Arg.Any<State>())
                .Returns(x => {
                    times.Add(DateTime.Now);
                    return HResult.Ok;
                });

            e0.setTimer(timerOwner, 100, 200);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            var startTime = DateTime.Now;
            Thread.Sleep(1000);
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.Ok));

            Assert.That(times.Count, Is.EqualTo(expectedTimes.Length));
            Console.WriteLine($"Start time={startTime:ss.fff}");

            // Check that the Event has been triggered by
            // 50 mSec delay and 100 mSec interval with an accuracy of less than 16 mSec.
            for(var i = 0; i < expectedTimes.Length; i++)
            {
                var time = startTime + TimeSpan.FromMilliseconds(expectedTimes[i]);
                Console.WriteLine($"  {times[i]:ss.fff} {time:ss.fff} diff={(times[i]-time).TotalMilliseconds}");
                Assume.That(times[i], Is.EqualTo(time).Within(TimeSpan.FromMilliseconds(32)));
                startTime = times[i];
            }
        }
    }
}
