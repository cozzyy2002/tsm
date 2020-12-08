using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using tsm_NET;

namespace StateMachine.NET.TestConsole
{
    class ShowErrors : IJob
    {
        void IJob.Start(IList<string> args)
        {
            foreach(HResult hr in Enum.GetValues(typeof(HResult)))
            {
                var error = new Error(hr);
                Console.WriteLine($"0x{(int)hr:x8} {hr}: {error.Message}");
            }
        }
    }
}
