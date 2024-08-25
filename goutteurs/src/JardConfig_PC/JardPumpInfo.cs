using System;
using System.Collections.Generic;
using System.Text;

namespace JardConfig
{
    public class JardPumpInfo
    {
        public bool forced;
        public bool enabled;
        public bool auto;
        public bool remote;
        public bool cmd_remote;
        public bool led;

        public int hour;
        public int minute;
        public int duration;
        public int days;

        public int timer;
    }
}
