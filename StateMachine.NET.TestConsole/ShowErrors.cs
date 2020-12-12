using System;
using System.Collections.Generic;
using System.Globalization;
using Error = tsm_NET.Generic.Error;

namespace StateMachine.NET.TestConsole
{
    class ShowErrors : IJob
    {
        void IJob.Start(IList<string> args)
        {
            if (0 < args.Count)
            {
                // Parse argument as culture name such as `ja-JP`.
                Error.CultureInfo = new CultureInfo(args[0]);
            }
            //show<tsm_NET.HResult>();
            show<tsm_NET.Generic.HResult>();
        }

        void show<H>()
            where H : Enum
        {
            Console.WriteLine($"\n---- Enumerating {typeof(H)} ----");
            foreach(var hr in Enum.GetValues(typeof(H)))
            {
                var error = new Error((int)hr);
                Console.Write($"0x{(int)hr:x8} {hr}: {error.Message}");
            }
        }
    }
}
