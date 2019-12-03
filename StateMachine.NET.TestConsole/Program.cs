using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using tsm_NET.Generic;

namespace StateMachine.NET.TestConsole
{
    class Program
    {
        static void Main(string[] args)
        {
            var context = new Context();
            context.setup(new State());
            context.waitReady(new TimeSpan(0, 0, 1));

            while (true)
            {
                Console.Write("> ");
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
        }
    }

    class Context : tsm_NET.Generic.Context<Event, State>
    {
    }

    class State : tsm_NET.Generic.State<Context, Event, State>
    {
        public State(State masterState = null) : base(masterState)
        {
            generation = (masterState != null) ? masterState.generation + 1 : 0;
        }

        public override HResult entry(Context context, Event @event, State previousState)
        {
            Console.WriteLine(string.Format("{0}: {1}", @event, this));
            return HResult.Ok;
        }

        public override HResult handleEvent(Context context, Event @event, ref State nextState)
        {
            if(generation < @event.NextGeneration)
            {
                // Go to next State with current State as master state.
                // This path continues until generation equals to Event.NextGeneration.
                nextState = new State(this);
                context.triggerEvent(new Event(@event.NextGeneration));
            }
            else if(@event.NextGeneration < generation)
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
            return string.Format("State: generation={0}", generation);
        }

        int generation;
    }

    class Event : tsm_NET.Generic.Event<Context>
    {
        public Event(int nextGeneration)
        {
            NextGeneration = nextGeneration;
        }

        public override string ToString()
        {
            return string.Format("Event: NextGeneration={0}", NextGeneration);
        }

        public readonly int NextGeneration;
    }
}
