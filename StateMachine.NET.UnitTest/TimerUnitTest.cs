using NSubstitute;
using NSubstitute.ReceivedExtensions;
using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace NET.TimerUnitTest
{
    using Context = Testee.Generic.AsyncContext;
    using Event = Testee.Generic.AsyncEvent;
    using State = Testee.Generic.AsyncState;
    using HResult = tsm_NET.Generic.HResult;

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

    [TestFixture(true)]     // Use Context as TimerClient
    [TestFixture(false)]    // Use State as TimerClient
    class TimerClientUnitTest : TimerUnitTest
    {
        public TimerClientUnitTest(bool isTimerClientContext)
        {
            this.isTimerClientContext = isTimerClientContext;
        }

        [SetUp]
        public new void SetUp()
        {
            // Set timerClient according to argument of TestFixture attribute.
            timerClient = isTimerClientContext ? (tsm_NET.TimerClient)context : mockState0;
        }

        bool isTimerClientContext;
        tsm_NET.TimerClient timerClient;

        [Test]
        public void CancelTimerReturnValueTest()
        {
            // Timer is not set.
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.IllegalMethodCall));
            e0.setDelayTimer(timerClient, TimeSpan.Zero);
            // Timer is not started.
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.False));
        }

        // One-shot timer
        [Test]
        public void OneShotTimer()
        {
            Console.WriteLine($"One-shot timer test using {timerClient}");

            e0.setDelayTimer(timerClient, 100);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerClient.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);
            Thread.Sleep(100);

            // Timer should be stopped.
            events = timerClient.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(0));

            // Timer event should have been handled once.
            mockState0.Received()
                .handleEvent(context, e0, ref Arg.Any<State>());
        }

        // Cancel one-shot timer
        [Test]
        public void CancelOneShotTimer()
        {
            Console.WriteLine($"Cancel one-Shot timer test using {timerClient}");

            e0.setDelayTimer(timerClient, 100);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerClient.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.Ok));

            // Timer should be canceled.
            events = timerClient.PendingEvents;
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
            Console.WriteLine($"Interval timer test using {timerClient}");

            e0.setTimer(timerClient, 100, 200);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerClient.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);
            Thread.Sleep(500);
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.Ok));

            // Timer should be stopped.
            events = timerClient.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(0));

            // Timer event should have been handled 3 times(Delay x 1 + Interval x 2).
            mockState0.Received(3)
                .handleEvent(context, e0, ref Arg.Any<State>());
        }

        [Test]
        public void CancelbyShutdown()
        {
            e0.setDelayTimer(timerClient, 100);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(50);

            // Timer should be working.
            var events = timerClient.PendingEvents;
            Assert.That(events.Count, Is.EqualTo(1));
            Assert.That(events.Contains(e0), Is.True);

            Assert.That(context.shutdown(), Is.EqualTo(HResult.Ok));
            Thread.Sleep(100);

            if (timerClient == context)
            {
                // Timer should be stopped.
                events = timerClient.PendingEvents;
                Assert.That(events.Count, Is.EqualTo(0));
            }
            else
            {
                // NOTE:
                // `timerClient.PendingEvents` can not be called when timerClient is State object
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
            e0.setTimer(timerClient, 50, 100);
            Assert.That(context.handleEvent(e0), Is.EqualTo(HResult.Ok));
            Assert.That(timerClient.PendingEvents.Count, Is.EqualTo(1));
            Thread.Sleep(500);
            Assert.That(timerClient.PendingEvents.Count, Is.EqualTo(0));
        }

        [Test]
        public void TimerAccuracyTest()
        {
            var expectedTimes = new int[] { 50, 100, 100, 100, 100 };
            var times = new List<DateTime>();
            mockState0.handleEvent(context, e0, ref Arg.Any<State>())
                .Returns(x => {
                    times.Add(DateTime.Now);
                    return HResult.Ok;
                });

            var startTime = DateTime.Now;
            e0.setTimer(timerClient, 50, 100);
            Assert.That(context.triggerEvent(e0), Is.EqualTo(HResult.Ok));
            Thread.Sleep(500);
            Assert.That(e0.cancelTimer(), Is.EqualTo(HResult.Ok));

            Assert.That(times.Count, Is.EqualTo(expectedTimes.Length));
            Console.WriteLine($"Start time={startTime:ss.fff}");

            // Check that the Event has been triggered by
            // 50 mSec delay and 100 mSec interval with an accuracy of less than 16 mSec.
            for(var i = 0; i < expectedTimes.Length; i++)
            {
                var time = startTime + TimeSpan.FromMilliseconds(expectedTimes[i]);
                Console.WriteLine($"  {times[i]:ss.fff} {time:ss.fff} diff={(times[i]-time).TotalMilliseconds}");
                Assert.That(times[i], Is.EqualTo(time).Within(TimeSpan.FromMilliseconds(16)));
                startTime = times[i];
            }
        }
    }
}
