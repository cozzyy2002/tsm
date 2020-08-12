using NSubstitute;
using NUnit.Framework;
using System;
using System.Collections;
using System.Threading;

namespace StateMachine.NET.AsyncUnitTest
{
    using Context = Testee.Generic.AsyncContext;
    using Event = Testee.Generic.AsyncEvent;
    using State = Testee.Generic.AsyncState;
    using HResult = tsm_NET.Generic.HResult;

    [TestFixture]
    class StateMachineAsyncUnitTest
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
            const int EventCount = 4;
            int ActualEventCount = 0;

            mockState.handleEvent(context, Arg.Any<Event>(), ref Arg.Any<State>())
                .Returns(x => {
                    var ev = x[1] as Event;
                    if(ev.Id == 0) { Thread.Sleep(100); }
                    Assert.That(ev.Id, Is.EqualTo(ActualEventCount++));
                    return HResult.Ok;
                });

            for(var i = 0; i < EventCount; i++)
            {
                var ev = new Event();
                ev.Id = i;
                Assert.That(context.triggerEvent(ev), Is.EqualTo(HResult.Ok));
            }
            Thread.Sleep(TimeSpan.FromSeconds(1));

            Assert.That(ActualEventCount, Is.EqualTo(EventCount));
        }

        public static IEnumerable PriorityValueTestData
        {
            get {
                yield return new TestCaseData(new int[] { 0, 0, 0, 0, 0 }, new int[] { 0, 1, 2, 3, 4 });    // Same priority.
                yield return new TestCaseData(new int[] { 0, -2, 0, 0, 0 }, new int[] { 0, 2, 3, 4, 1 });   // First is lower.
                yield return new TestCaseData(new int[] { 0, 0, 0, 0, 2 }, new int[] { 0, 4, 1, 2, 3 });    // Last is Higher.
                yield return new TestCaseData(new int[] { 0, 0, -2, 0, 0 }, new int[] { 0, 1, 3, 4, 2 });   // Middle is lower.
                yield return new TestCaseData(new int[] { 0, 0, 0, 2, 0 }, new int[] { 0, 3, 1, 2, 4 });    // Middle is higher.
                yield return new TestCaseData(new int[] { 0, 4, 3, 2, 1 }, new int[] { 0, 1, 2, 3, 4 });    // Priority order.
                yield return new TestCaseData(new int[] { 0, 1, 2, 3, 4 }, new int[] { 0, 4, 3, 2, 1 });	// Reverse priority order.
            }
        }

        // Specifick event priority.
        // State::HandleEvent() should be called by order of Event::Priority property.
        [TestCaseSource(typeof(StateMachineAsyncUnitTest), "PriorityValueTestData")]
        public void PriorityValueTest(int[] priorities, int[] sequences)
        {
            int EventCount = priorities.Length;
            int ActualEventCount = 0;

            mockState.handleEvent(context, Arg.Any<Event>(), ref Arg.Any<State>())
                .Returns(x => {
                    var ev = x[1] as Event;
                    if (ev.Id == 0) { Thread.Sleep(100); }
                    Assume.That(ev.Id, Is.EqualTo(sequences[ActualEventCount++]));
                    return HResult.Ok;
                });

            for (var i = 0; i < EventCount; i++)
            {
                var ev = new Event(priorities[i]);
                ev.Id = i;
                Assert.That(context.triggerEvent(ev), Is.EqualTo(HResult.Ok));
            }
            Thread.Sleep(TimeSpan.FromSeconds(1));

            Assert.That(ActualEventCount, Is.EqualTo(EventCount));
        }
    }
}
