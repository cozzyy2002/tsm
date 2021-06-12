using System;
using System.Collections.Generic;
using tsm_NET;

namespace StateMachine.NET.TestConsole
{
    class MemoryWeight : Common, IJob
    {
        void IJob.Start(IList<string> args)
        {
            State.MemoryWeight = 100;
            foreach (var arg in args)
            {
                int memoryWeight;
                if (int.TryParse(arg, out memoryWeight))
                {
                    // Integer argument is set to State.MemoryWeight property.
                    State.MemoryWeight = memoryWeight;
                }

                bool stateAutoDispose;
                if(bool.TryParse(arg, out stateAutoDispose))
                {
                    // Boolean argument is set to State.DefaultAutoDispose property.
                    State.DefaultAutoDispose = stateAutoDispose;
                }
            }

            Console.WriteLine($"Creating State object: MemoryWeight={State.MemoryWeight} MByte, AutoDispose={State.DefaultAutoDispose}.");

            var context = new Context();
            var initialState = new State();
            context.setup(initialState);
            context.StateMonitor = new StateMonitor<IContext, IEvent, IState>();

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
            if (!State.DefaultAutoDispose) { initialState.Dispose(); }

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
            // Construct 1-st State object.
            public State()
            {
                generation = 0;
                IsExitCalledOnShutdown = true;
            }

            // Construct Sub State object that inherits properties of Master State.
            public State(State masterState) : base(masterState)
            {
                generation = masterState.generation + 1;
                IsExitCalledOnShutdown = masterState.IsExitCalledOnShutdown;
                masterState.HasSubState = true;
            }

            public override HResult entry(Context context, Event @event, State previousState)
            {
                Console.WriteLine($"{Now} {this}.entry({@event}, {previousState})");

                var hr = HResult.Ok;
                if ((@event != null) && (generation < @event.NextGeneration))
                {
                    // Trigger event to go to next state.
                    // This path continues until generation equals to Event.NextGeneration.
                    hr = context.triggerEvent(@event);
                }
                return hr;
            }

            public override HResult exit(Context context, Event @event, State nextState)
            {
                Console.WriteLine($"{Now} {this}.exit({@event}, {nextState})");
                return HResult.Ok;
            }

            public override HResult handleEvent(Context context, Event @event, ref State nextState)
            {
                if (generation < @event.NextGeneration)
                {
                    // Go to next State with current State as master state.
                    // This path continues until generation equals to Event.NextGeneration.
                    nextState = new State(this);
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
                return $"State(generation={generation})";
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
                return $"Event(NextGeneration={NextGeneration})";
            }

            public readonly int NextGeneration;
        }

        class StateMonitor : StateMonitor<Context, Event, State>
        {
            public void onIdle(Context<Event, State> context)
            {
                Console.Write($"{Now} onIdle(): Total memory={GC.GetTotalMemory(true)} > ");
            }

            public void onEventTriggered(Context<Event, State> context, Event @event) { }
            public void onEventHandling(Context<Event, State> context, Event @event, State current) { }

            public void onStateChanged(Context<Event, State> context, Event @event, State previous, State next)
            {
                DisposeSubStates(next.generation, previous);
            }

            public void onTimerStarted(Context<Event, State> context, Event @event) { }
            public void onTimerStopped(Context<Event, State> context, Event @event, HResult hr) { }

            public void onWorkerThreadExit(Context<Event, State> context, HResult exitCode)
            {
                DisposeSubStates(-1, context.CurrentState);
            }
        }

        // Dispose State object(s) until State.generation is greater than specified generation.
        // If State.AutoDispose == true, this mehtod does nothing.
        static void DisposeSubStates(int genaration, State state)
        {
            if(!State.DefaultAutoDispose && (state != null) && !state.HasSubState)
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
