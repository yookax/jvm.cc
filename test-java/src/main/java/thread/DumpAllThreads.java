package thread;

import java.lang.management.ManagementFactory;
import java.lang.management.ThreadMXBean;
import java.lang.management.ThreadInfo;

class DumpAllThreads {
    public static void main(String[] args) {
        ThreadMXBean mxb = ManagementFactory.getThreadMXBean();
        ThreadInfo[] infos = mxb.dumpAllThreads(false, false);
        for (ThreadInfo info : infos) {
            System.out.println("[" + info.getThreadId() + "] " + info.getThreadName());
        }
    }
}