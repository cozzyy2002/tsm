using System;
using System.Collections.Generic;

using Error = tsm_NET.Error;

namespace StateMachine.NET.TestConsole
{
    class ShowErrors : IJob
    {
        void IJob.Start(IList<string> args)
        {
            show<tsm_NET.HResult>();
            show<tsm_NET.Generic.HResult>();
        }

        void show<H>()
            where H : Enum
        {
            Console.WriteLine($"---- Enumerating {typeof(H)} ----");
            foreach(var hr in Enum.GetValues(typeof(H)))
            {
                var error = new Error((int)hr);
                Console.Write($"0x{(int)hr:x8} {hr}: {error.Message}");
            }
        }
    }
}

