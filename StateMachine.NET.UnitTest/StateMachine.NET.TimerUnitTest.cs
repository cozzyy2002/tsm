using NSubstitute;
using NSubstitute.ReceivedExtensions;
using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace StateMachine.NET.TimerUnitTest
{
    using Context = Testee.Generic.AsyncContext;
    using Event = Testee.Generic.AsyncEvent;
    using State = Testee.Generic.AsyncState;
    using HResult = tsm_NET.Generic.HResult;

    [TestFixture]
    class StateMachineTimerUnitTest
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
            Assert.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));
            mockState0.Received().entry(context, null, null);
        }

        [TearDown]
        public void TearDown()
        {
            Assert.That(context.shutdown(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));
            mockState0.Received().exit(context, null, null);
        }

        protected Context context;
        protected Event e0, e1;
        protected State mockState0, mockState1;
    }

    [TestFixture(true)]     // Use Context as TimerClient
    [TestFixture(false)]    // Use State as TimerClient
    class StateMachineTimerClientUnitTest : StateMachineTimerUnitTest
    {
        public StateMachineTimerClientUnitTest(bool isTimerClientContext)
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

        // One-shot timer
        [Test]
        public void OneShotTimer()
        {
            Console.WriteLine($"One-shot timer test using {timerClient}");

            e0.setDelayTimer(timerClient, TimeSpan.FromMilliseconds(100));
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

            e0.setDelayTimer(timerClient, TimeSpan.FromMilliseconds(100));
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

            e0.setTimer(timerClient, TimeSpan.FromMilliseconds(100), TimeSpan.FromMilliseconds(200));
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
    }
}
