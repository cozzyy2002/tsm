using NSubstitute;
using NUnit.Framework;
using System;
using System.Threading;
using tsm_NET;

namespace StateMachine.NET.UnitTest
{
    class Class2
    {
        [TearDown]
        public void TearDown()
        {
            // Wait for worker thread of StateMachine to process events and to terminate.
            Thread.Sleep(100);
        }

        [Test]
        public void test1()
        {
            var mockState = Substitute.For<State>();
            mockState
                .entry(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>())
                .Returns(x =>
                {
                    return HResult.Ok;
                });
            var c = new Context();
            Assert.That(c.CurrentState, Is.EqualTo(null), "error");

        }
    }
}
