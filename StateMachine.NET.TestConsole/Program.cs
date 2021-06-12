using System;
using System.Collections.Generic;
using tsm_NET;

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
            if (0 < args.Count) {
                int.TryParse((string)args[0], out sel);
                args.RemoveAt(0);
            }

            switch (sel)
            {
            case 1:
                job = new MemoryWeight();
                break;
            case 2:
                job = new EventTimer();
                break;
            case 3:
                job = new ShowErrors();
                break;
            default:
                Console.WriteLine("Specify number 1=MemoryWeight, 2=EventTimer, 3=ShowErrors");
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

        public static string Now { get { return (DateTime.Now - start).ToString(@"mm\.ss\.fff"); } }
    }

    class StateMonitor<Context, Event, State> : tsm_NET.StateMonitor<Context, Event, State>
        where Context : IContext
        where Event : IEvent
        where State : IState
    {
        public override void onEventHandling(Context context, Event @event, State current)
        {
            Console.WriteLine($"{Now} onEventHandling({@event}, {current})");
        }

        public override void onEventTriggered(Context context, Event @event)
        {
            Console.WriteLine($"{Now} onEventTriggered({@event})");
        }

        public override void onIdle(Context context)
        {
            Console.WriteLine($"{Now} onIdle()");
        }

        public override void onStateChanged(Context context, Event @event, State previous, State next)
        {
            Console.WriteLine($"{Now} onStateChanged({@event}, {previous}, {next})");
        }

        public override void onTimerStarted(Context context, Event @event)
        {
            Console.WriteLine($"{Now} onTimerStarted({@event})");
        }

        public override void onTimerStopped(Context context, Event @event, HResult hr)
        {
            Console.WriteLine($"{Now} onTimerStarted({@event}, {hr,08:x})");
        }

        public override void onWorkerThreadExit(Context context, HResult exitCode)
        {
            Console.WriteLine($"{Now} onWorkerThreadExit({exitCode})");
        }

        string Now { get {return Common.Now;} }
    }
}
