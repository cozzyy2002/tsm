﻿using System;
using System.Collections.Generic;
using System.Security.AccessControl;
using tsm_NET.Generic;

namespace StateMachine.NET.TestConsole
{
    class MemoryWeight : Common, IJob
    {
        static bool StateAutoDispose = false;

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

            State.DefaultAutoDispose = StateAutoDispose;

            Console.WriteLine($"Allocate {State.MemoryWeight} MByte for each State object.");

            var context = new Context();
            var initialState = new State();
            context.setup(initialState);
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
            if (!StateAutoDispose) { initialState.Dispose(); }

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
                if (masterState != null)
                {
                    generation = masterState.generation + 1;
                    masterState.HasSubState = true;
                }
                else
                {
                    generation = 0;
                }
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

            public override string ToString()
            {
                return string.Format("State({0}): generation={1}", SequenceNumber, generation);
            }

            public readonly int generation;
            public bool HasSubState = false;
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
                Console.Write($"{Now} onIdle(): Total memory={GC.GetTotalMemory(true)} > ");
            }

            public override void onStateChanged(Context<Event, State> context, Event @event, State previous, State next)
            {
                DisposeSubStates(next.generation, previous);
            }

            public override void onWorkerThreadExit(Context<Event, State> context, HResult exitCode)
            {
                DisposeSubStates(-1, context.CurrentState);
            }
        }

        // Dispose State object(s) until State.generation is greater than specified generation.
        static void DisposeSubStates(int genaration, State state)
        {
            if(!StateAutoDispose && (state != null) && !state.HasSubState)
            {
                // At this point:
                //   State object should be disposed explicitly.
                //   The state is the end of MasterState/SubState chain.
                while(genaration < state.generation)
                {
                    var _state = state.MasterState;
                    state.Dispose();
                    if (_state != null)
                    {
                        state = _state;
                        state.HasSubState = false;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}
