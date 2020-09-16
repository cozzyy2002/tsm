using System;
using System.Collections.Generic;
using tsm_NET.Generic;

namespace StateMachine.NET.TestConsole
{
    interface IJob
    {
        void Start(IList<string> args);
    };

    class Program
    {
        static void Main(string[] _args)
        {
            IJob job = null;
            int sel = 0;
            var args = new List<string>(_args);
            if(0 < args.Count) {
                int.TryParse((string)args[0], out sel);
                args.RemoveAt(0);
            }

            switch(sel)
            {
            case 1:
                job = new MemoryWeight();
                break;
            case 2:
                job = new EventTimer();
                break;
            default:
                Console.WriteLine("Specify number 1=MemoryWeight, 2=EventTimer");
                return;
            }
            job.Start(args);
        }
    }

    class Common
    {
        static Common()
        {
            start = DateTime.Now;
        }

        private static DateTime start;

        protected static string Now { get { return (DateTime.Now - start).ToString(@"mm\.ss\.fff"); } }
    }

    class StateMonitor<Event, State> : Common, IStateMonitor<Event, State>
    {
        public virtual void onEventHandling(Context<Event, State> context, Event @event, State current)
        {
            Console.WriteLine($"{Now} onEventHandling({@event}, {current})");
        }

        public virtual void onEventTriggered(Context<Event, State> context, Event @event)
        {
            Console.WriteLine($"{Now} onEventTriggered({@event})");
        }

        public virtual void onIdle(Context<Event, State> context)
        {
            Console.WriteLine($"{Now} onIdle()");
        }

        public virtual void onStateChanged(Context<Event, State> context, Event @event, State previous, State next)
        {
            Console.WriteLine($"{Now} onStateChanged({@event}, {previous}, {next})");
        }

        public virtual void onTimerStarted(Context<Event, State> context, Event @event)
        {
            Console.WriteLine($"{Now} onTimerStarted({@event})");
        }

        public void onTimerStopped(Context<Event, State> context, Event @event, HResult hr)
        {
            Console.WriteLine($"{Now} onTimerStarted({@event}, {hr,08:x})");
        }

        public virtual void onWorkerThreadExit(Context<Event, State> context, HResult exitCode)
        {
            Console.WriteLine($"{Now} onWorkerThreadExit({exitCode})");
        }
    }
}
