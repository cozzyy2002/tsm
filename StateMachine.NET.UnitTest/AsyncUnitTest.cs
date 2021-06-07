using NSubstitute;
using NUnit.Framework;
using System;
using System.Collections;
using System.Threading;

namespace NET.AsyncUnitTest
{
    using Context = Testee.Generic.AsyncContext;
    using Event = Testee.Generic.AsyncEvent;
    using State = Testee.Generic.AsyncState;
    using HResult = tsm_NET.HResult;

    [TestFixture]
    class AsyncUnitTest
    {
        [SetUp]
        public void SetUp()
        {
            context = new Context();
            mockState = Substitute.For<State>();
            mockState.IsExitCalledOnShutdown = true;
            Assert.That(context.setup(mockState), Is.EqualTo(HResult.Ok));
            Assert.That(context.waitReady(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));
            mockState.Received().entry(context, null, null);
        }

        [TearDown]
        public void TearDown()
        {
            Assert.That(context.shutdown(TimeSpan.FromSeconds(1)), Is.EqualTo(HResult.Ok));
            mockState.Received().exit(context, null, null);
        }

        Context context;
        State mockState;

        // Default event priority.
        // State::handleEvent() should be called by order of AsyncContext::triggerEvent().
        [Test]
        public void DefaultPriorityTest()
        {
            // Expected sequence: Order that each event is triggered.
            var sequences = new int[] { 0, 1, 2, 3 };
            var EventCount = sequences.Length;
            var ActualEventCount = 0;
            var ActualSequences = new int[EventCount];
            var AllEventsHandled = new EventWaitHandle(false, EventResetMode.ManualReset);

            // Create and trigger events that have same priority.
            var firstEvent = Substitute.For<Event>();
            firstEvent.preHandle(context)
                .Returns(x => {
                    foreach(var id in sequences)
                    {
                        var ev = new Event();
                        ev.Id = id;
                        Assert.That(context.triggerEvent(ev), Is.EqualTo(HResult.Ok));
                    }
                    // This event does not have to be handled.
                    return HResult.False;
                });
            Assert.That(context.triggerEvent(firstEvent), Is.EqualTo(HResult.Ok));

            // Save ID of passed event to ActualSequences array.
            mockState.handleEvent(context, Arg.Any<Event>(), ref Arg.Any<State>())
                .Returns(x => {
                    var ev = x[1] as Event;
                    ActualSequences[ActualEventCount++] = ev.Id;
                    if(ActualEventCount == EventCount) { AllEventsHandled.Set(); }
                    return HResult.Ok;
                });

            // Wait for last event to be handled.
            Assert.That(AllEventsHandled.WaitOne(TimeSpan.FromSeconds(1)), Is.True);

            // Check count and sequence of handled events.
            Assert.That(ActualEventCount, Is.EqualTo(EventCount));
            Assert.That(ActualSequences, Is.EqualTo(sequences));
        }

        public static IEnumerable PriorityValueTestData
        {
            get {
                yield return new TestCaseData(new int[] { 0, 0, 0, 0 }, new int[] { 0, 1, 2, 3 });      // Same priority.
                yield return new TestCaseData(new int[] { -2, 0, 0, 0 }, new int[] { 1, 2, 3, 0 });     // First is lower.
                yield return new TestCaseData(new int[] { 0, 0, 0, 2 }, new int[] { 3, 0, 1, 2 });      // Last is Higher.
                yield return new TestCaseData(new int[] { 0, -2, 0, 0 }, new int[] { 0, 2, 3, 1 });     // Middle is lower.
                yield return new TestCaseData(new int[] { 0, 0, 2, 0 }, new int[] { 2, 0, 1, 3 });      // Middle is higher.
                yield return new TestCaseData(new int[] { 4, 3, 2, 1 }, new int[] { 0, 1, 2, 3 });      // Priority order.
                yield return new TestCaseData(new int[] { 1, 2, 3, 4 }, new int[] { 3, 2, 1, 0 });      // Reverse priority order.
            }
        }

        // Various event priority.
        // State::HandleEvent() should be called by order of Event::Priority property.
        [TestCaseSource(typeof(AsyncUnitTest), "PriorityValueTestData")]
        public void PriorityValueTest(int[] priorities, int[] sequences)
        {
            int EventCount = priorities.Length;
            int ActualEventCount = 0;
            int[] ActualSequences = new int[EventCount];
            EventWaitHandle AllEventsHandled = new EventWaitHandle(false, EventResetMode.ManualReset);

            // Create and trigger events according to priorities array.
            Event firstEvent = Substitute.For<Event>();
            firstEvent.preHandle(context)
                .Returns(x => {
                    for (var i = 0; i < EventCount; i++)
                    {
                        var ev = new Event(priorities[i]);
                        ev.Id = i;
                        Assert.That(context.triggerEvent(ev), Is.EqualTo(HResult.Ok));
                    }
                    // This event does not have to be handled.
                    return HResult.False;
                });
            Assert.That(context.triggerEvent(firstEvent), Is.EqualTo(HResult.Ok));

            // Save ID of passed event to ActualSequences array.
            mockState.handleEvent(context, Arg.Any<Event>(), ref Arg.Any<State>())
                .Returns(x => {
                    var ev = x[1] as Event;
                    ActualSequences[ActualEventCount++] = ev.Id;
                    if (ActualEventCount == EventCount) { AllEventsHandled.Set(); }
                    return HResult.Ok;
                });

            // Wait for last event to be handled.
            Assert.That(AllEventsHandled.WaitOne(TimeSpan.FromSeconds(1)), Is.True);

            // Check count and sequence of handled events.
            Assert.That(ActualEventCount, Is.EqualTo(EventCount));
            Assert.That(ActualSequences, Is.EqualTo(sequences));
        }
    }
}
