﻿using NSubstitute;
using NUnit.Framework;
using System;
using System.Threading;

namespace StateMachine.NET.UnitTest.Generic
{
    using HResult = tsm_NET.HResult;

    public class Context : tsm_NET.Generic.Context<Event, State>
    {
    }

    public class Event : tsm_NET.Generic.Event<Context>
    {

    }

    public class State : tsm_NET.Generic.State<Context, Event, State>
    {
        public override HResult handleEvent(Context context, Event @event, ref State nextState)
        {
            Console.WriteLine($"{this.GetType().ToString()}.handleEvent() is called.");
            Assert.That(context, Is.TypeOf<Context>());
            return HResult.Ok;
        }
    }

    [TestFixture]
    public class Class1
    {
        [TearDown]
        public void TearDown()
        {
            // Wait for worker thread of StateMachine to process events and to terminate.
            //Thread.Sleep(1000);
        }

        [Test]
        public void test1()
        {
            var mockState = Substitute.For<State>();

            var c = new Context();
            Assert.That(c.CurrentState, Is.EqualTo(null), "Context has no initial state when created.");

            mockState
                .entry(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>())
                .Returns(x =>
                {
                    Console.Error.WriteLine($"{this.GetType().ToString()}.handleEvent() is called.");
                    return tsm_NET.HResult.Ok;
                });

            Assert.That(c.setup(mockState), Is.EqualTo(HResult.Ok));
            Assert.That(c.CurrentState, Is.EqualTo(mockState), "Context has argument of setup() as initial state.");

            Thread.Sleep(1000);

            mockState.Received()
                .entry(Arg.Any<Context>(), Arg.Any<Event>(), Arg.Any<State>());
            mockState.DidNotReceive()
                .handleEvent(Arg.Any<Context>(), Arg.Any<Event>(), ref Arg.Any<State>());
        }
    }

    public interface ICommand
    {
        void Execute();
        event EventHandler Executed;
    }

    public class SomethingThatNeedsACommand
    {
        ICommand command;
        public SomethingThatNeedsACommand(ICommand command)
        {
            this.command = command;
        }
        public void DoSomething() { command.Execute(); }
        public void DontDoAnything() { }
    }

    public class Sample
    {
        [Test]
        public void Should_execute_command()
        {
            //Arrange
            var command = Substitute.For<ICommand>();
            var something = new SomethingThatNeedsACommand(command);
            //Act
            something.DoSomething();
            //Assert
            command.Received().Execute();
        }
    }
}
