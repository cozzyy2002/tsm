using NUnit.Framework;

namespace StateMachine.NET.UnitTest
{
    public class Context : tsm_NET.Generic.Context<Event, State>
    {
    }

    public class Event : tsm_NET.Generic.Event<Context>
    {

    }

    public class State : tsm_NET.Generic.State<Context, Event, State>
    {

    }

    [TestFixture]
    public class Class1
    {
        [Test]
        public void test1()
        {
            var c = new Context();
            Assert.That(c.CurrentState, Is.EqualTo(null), "error");
        }
    }
}
