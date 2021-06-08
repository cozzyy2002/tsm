using System;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Threading;
using tsm_NET;

namespace StateMachine.NET.TestConsole
{
    class EventTimer : Common, IJob
    {
        void IJob.Start(IList<string> args)
        {
            var context = new Context();
            context.StateMonitor = new StateMonitor<IContext, IEvent, IState>();

            context.Count = 2;
            var initialState = new InitialState();
            context.setup(initialState);

            var @event = new Event("Initial");
            @event.setTimer(initialState, TimeSpan.FromSeconds(1), TimeSpan.FromSeconds(2));
            context.triggerEvent(@event);
            Thread.Sleep(100);
            var events = initialState.PendingEvents;
            Console.WriteLine($"Pending events={events.Count}");
            foreach(Event e in events) { Console.WriteLine($"  {e}"); }

            // Wait for onHandleEvent of next state to be called 2 times.
            while (context.CurrentState == initialState) { Thread.Sleep(100); }
            var wait = context.CompleteEvent.WaitOne(TimeSpan.FromSeconds(8));
            Console.WriteLine($"{Now} Waiting event done: Event signaled={wait}");

            context.shutdown();
        }

        class Context : AsyncContext<Event, State>
        {
            public Context() : base(false) { }
            public int Count { get; set; }
            public EventWaitHandle CompleteEvent { get; set; } = new EventWaitHandle(false, EventResetMode.ManualReset);

            public override string ToString() { return $"Context: Count={Count}"; }
        }

        class InitialState : State
        {
            public InitialState() : base("Initial") { }
            public override HResult handleEvent(Context context, Event @event, ref State nextState)
            {
                if (--context.Count == 0) { nextState = new NextState(); }
                return HResult.Ok;
            }
        }

        class NextState : State
        {
            public NextState() : base("Next") { }
            public override HResult entry(Context context, Event @event, State previousState)
            {
                var _event = new Event("Next");
                _event.setTimer(this, @event.DelayTime, @event.IntervalTime);
                context.triggerEvent(_event);
                return HResult.Ok;
            }
            public override HResult handleEvent(Context context, Event @event, ref State nextState)
            {
                context.Count++;
                if(2 < context.Count) { context.CompleteEvent.Set(); }
                return HResult.Ok;
            }
        }

        class State : State<Context, Event, State>
        {
            public State(string name)
            {
                Name = $"State: {name}";
            }

            public string Name { get; protected set; }
            public override string ToString() { return Name; }
        }

        class Event : Event<Context>
        {
            public Event(string name) { Name = name; }

            public string Name { get; protected set; }
            public override string ToString()
            {
                return $"Event: {Name}, Delay={DelayTime.TotalSeconds}, Interval={IntervalTime.TotalSeconds}";
            }
        }

        class StateMonitor : StateMonitor<Context, Event, State>
        {
            void print(string args, [CallerMemberName] string caller="")
            {
                Console.WriteLine($"{Now} [{Context.CurrentThread}] {caller}({args})");
            }

            public void onEventHandling(Context<Event, State> context, Event @event, State current)
            {
                print($"{context}, {@event}, {current}");
            }

            public void onEventTriggered(Context<Event, State> context, Event @event)
            {
                print($"{context}, {@event}");
            }

            public void onIdle(Context<Event, State> context)
            {
                print($"{context}");
            }

            public void onStateChanged(Context<Event, State> context, Event @event, State previous, State next)
            {
                print($"{context}, {@event}, {previous}, {next}");
            }

            public void onTimerStarted(Context<Event, State> context, Event @event)
            {
                print($"{context}, {@event}");
            }

            public void onTimerStopped(Context<Event, State> context, Event @event, HResult hr)
            {
                print($"{context}, {@event}, {hr,08:x}");
            }

            public void onWorkerThreadExit(Context<Event, State> context, HResult exitCode)
            {
                print($"{context}, ExitCode={exitCode}");
            }
        }
    }
}
