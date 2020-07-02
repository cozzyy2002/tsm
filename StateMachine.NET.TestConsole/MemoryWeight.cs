using System;
using System.Collections.Generic;
using tsm_NET.Generic;

namespace StateMachine.NET.TestConsole
{
    class MemoryWeight : Common, IJob
    {
        void IJob.Start(IList<string> args)
        {
            State.MemoryWeight = 100;
            if (0 < args.Count)
            {
                int memoryWeight;
                if (int.TryParse(args[0], out memoryWeight))
                {
                    State.MemoryWeight = memoryWeight;
                }
            }

            Console.WriteLine($"Allocate {State.MemoryWeight} MByte for each State object.");

            var context = new Context();
            context.setup(new State());
            context.StateMonitor = new StateMonitor();

            while (true)
            {
                var str = Console.ReadLine();
                if (string.IsNullOrEmpty(str)) { break; }

                int nextGeneration;
                if (int.TryParse(str, out nextGeneration))
                {
                    var e = new Event(nextGeneration);
                    context.triggerEvent(e);
                }
                else
                {

                }
            }

            var hr = context.shutdown();
            HResult hrExitCode;
            context.getAsyncExitCode(out hrExitCode);
            Console.WriteLine($"{Now} Context.shutdown() returns {hr}, Worker thread returns {hrExitCode}.\nKey in [Enter] to exit.");
            Console.ReadLine();
        }

        class Context : AsyncContext<Event, State>
        {
        }

        class State : State<Context, Event, State>
        {
            public State(State masterState = null) : base(masterState)
            {
                generation = (masterState != null) ? masterState.generation + 1 : 0;
            }

            public override HResult entry(Context context, Event @event, State previousState)
            {
                Console.WriteLine(string.Format($"{Now} {0}: {1}", @event, this));
                return HResult.Ok;
            }

            public override HResult handleEvent(Context context, Event @event, ref State nextState)
            {
                if (generation < @event.NextGeneration)
                {
                    // Go to next State with current State as master state.
                    // This path continues until generation equals to Event.NextGeneration.
                    nextState = new State(this);
                    context.triggerEvent(new Event(@event.NextGeneration));
                }
                else if (@event.NextGeneration < generation)
                {
                    // Go to master state whose generation is Event.NextGeneration.
                    var state = this;
                    while ((state != null) && (@event.NextGeneration < state.generation)) { state = state.MasterState; }
                    nextState = state;
                }

                return HResult.Ok;
            }

            public override HResult exit(Context context, Event @event, State nextState)
            {
                if (MasterState.generation == @event.NextGeneration)
                {
                    Console.WriteLine($"{Now} Force a garbage collection.");
                    GC.Collect();
                }
                return HResult.Ok;
            }

            public override string ToString()
            {
                return string.Format("State({0}): generation={1}", SequenceNumber, generation);
            }

            int generation;
        }

        class Event : Event<Context>
        {
            public Event(int nextGeneration)
            {
                NextGeneration = nextGeneration;
            }

            public override string ToString()
            {
                return string.Format("Event({0}): NextGeneration={1}", SequenceNumber, NextGeneration);
            }

            public readonly int NextGeneration;
        }

        class StateMonitor : StateMonitor<Event, State>
        {
            public override void onIdle(Context<Event, State> context)
            {
                base.onIdle(context);
                Console.Write($"{Now} {0} > ", GC.GetTotalMemory(true));
            }
        }
    }
}
