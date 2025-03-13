#include "vmdef.h"
#include "jmm.h"

import runtime;
import object;
import class_loader;

using namespace std;

jint JNICALL Jmm_GetVersion(JNIEnv *env) {
    return JMM_VERSION;
}

jint JNICALL Jmm_GetOptionalSupport(JNIEnv *env, jmmOptionalSupport *support) {
    TRACE("Jmm_GetOptionalSupport(env=%p, support=%p)", env, support);

    if(support == nullptr)
        return -1;

    support->isLowMemoryDetectionSupported         = 0;
    support->isCompilationTimeMonitoringSupported  = 0;
    support->isThreadContentionMonitoringSupported = 0;
    support->isCurrentThreadCpuTimeSupported       = 0;
    support->isOtherThreadCpuTimeSupported         = 0;
    support->isObjectMonitorUsageSupported         = 0;
    support->isSynchronizerUsageSupported          = 0;

    return 0;
}

jint JNICALL Jmm_GetThreadInfo(JNIEnv *env, jlongArray ids, jint maxDepth, jobjectArray infoArray) {
    unimplemented
}

jobjectArray JNICALL Jmm_GetMemoryPools(JNIEnv* env, jobject mgr) {
    unimplemented
}

jobjectArray JNICALL Jmm_GetMemoryManagers(JNIEnv* env, jobject pool) {
    unimplemented
}

jobject JNICALL Jmm_GetMemoryPoolUsage(JNIEnv* env, jobject pool) {
    unimplemented
}

jobject JNICALL Jmm_GetPeakMemoryPoolUsage(JNIEnv* env, jobject pool) {
    unimplemented
}

jlong JNICALL Jmm_GetOneThreadAllocatedMemory(JNIEnv *env, jlong thread_id) {
    unimplemented
}

void JNICALL Jmm_GetThreadAllocatedMemory(JNIEnv *env, jlongArray ids, jlongArray sizeArray) {
    unimplemented
}

jobject JNICALL Jmm_GetMemoryUsage(JNIEnv* env, jboolean heap) {
    unimplemented
}

jlong JNICALL Jmm_GetLongAttribute(JNIEnv *env, jobject obj, jmmLongAttribute att) {
    unimplemented
}

jboolean JNICALL Jmm_GetBoolAttribute(JNIEnv *env, jmmBoolAttribute att) {
    TRACE("Jmm_GetBoolAttribute(env=%p, att=%d)", env, att);

    switch (att) {
        case JMM_VERBOSE_GC:
        case JMM_VERBOSE_CLASS:
        case JMM_THREAD_CONTENTION_MONITORING:
        case JMM_THREAD_CPU_TIME:
        case JMM_THREAD_ALLOCATED_MEMORY:
            break;
        default:
            unimplemented; // Unknown attribute
            break;
    }

    return JNI_FALSE;
}

jboolean JNICALL Jmm_SetBoolAttribute(JNIEnv *env, jmmBoolAttribute att, jboolean flag) {
    unimplemented
}

jint JNICALL Jmm_GetLongAttributes(JNIEnv *env, jobject obj,
                                jmmLongAttribute* atts, jint count, jlong* result) {
    unimplemented
}

jobjectArray JNICALL Jmm_FindCircularBlockedThreads(JNIEnv *env) {
    unimplemented
}

// Not used in JDK 6 or JDK 7
jlong JNICALL Jmm_GetThreadCpuTime(JNIEnv *env, jlong thread_id) {
    unimplemented
}

jobjectArray JNICALL Jmm_GetVMGlobalNames (JNIEnv *env) {
    unimplemented
}

jint JNICALL Jmm_GetVMGlobals(JNIEnv *env, jobjectArray names, jmmVMGlobal *globals, jint count){
    unimplemented
}

jint JNICALL Jmm_GetInternalThreadTimes(JNIEnv *env, jobjectArray names, jlongArray times){
    unimplemented
}

jboolean JNICALL Jmm_ResetStatistic(JNIEnv *env, jvalue obj, jmmStatisticType type) {
    unimplemented
}

void JNICALL Jmm_SetPoolSensor(JNIEnv *env, jobject pool, jmmThresholdType type, jobject sensor) {
    unimplemented
}

jlong JNICALL Jmm_SetPoolThreshold(JNIEnv *env, jobject pool, 
                                    jmmThresholdType type, jlong threshold) {
    unimplemented
}

jobject JNICALL Jmm_GetPoolCollectionUsage(JNIEnv* env, jobject pool) {
    unimplemented
}

jint JNICALL Jmm_GetGCExtAttributeInfo(JNIEnv *env, jobject mgr, 
                                        jmmExtAttributeInfo *ext_info, jint count) {
    unimplemented
}

void JNICALL Jmm_GetLastGCStat(JNIEnv *env, jobject mgr, jmmGCStat *gc_stat) {
    unimplemented
}

jlong JNICALL Jmm_GetThreadCpuTimeWithKind(JNIEnv *env, jlong thread_id, jboolean user_sys_cpu_time) {
    unimplemented
}

void JNICALL Jmm_GetThreadCpuTimesWithKind(JNIEnv *env, jlongArray ids, 
                                        jlongArray timeArray, jboolean user_sys_cpu_time) {
    unimplemented
}

jint JNICALL Jmm_DumpHeap0(JNIEnv *env, jstring outputfile, jboolean live) {
    unimplemented
}

jobjectArray JNICALL Jmm_FindDeadlocks(JNIEnv *env, jboolean object_monitors_only) {
    unimplemented
}

void JNICALL Jmm_SetVMGlobal(JNIEnv *env, jstring flag_name, jvalue  new_value) {
    unimplemented
}

// lockedMonitors if true, dump all locked monitors.
// lockedSynchronizers if true, dump all locked ownable synchronizers.
jobjectArray JNICALL Jmm_DumpThreads(JNIEnv *env, jlongArray _ids, jboolean lockedMonitors, 
                                        jboolean lockedSynchronizers, jint maxDepth) {
    TRACE("Jmm_DumpThreads(env=%p)", env); // todo

    // jarrRef ids = (jarrRef) _ids; // could be NULL

    Class *c = load_boot_class("java/lang/management/ThreadInfo"); // in java.management module
    init_class(c);

    jref *thread_infos = new jref[g_all_java_thread.size()];
    int count = 0;

    for (Thread *t: g_all_java_thread) {
        jref o = t->tobj;
        if (o != nullptr && java_lang_Thread::isAlive(o)) {
            jref name = java_lang_Thread::getName(o);
            jlong tid = java_lang_Thread::getTid(o);
            
            jref thread_info = Allocator::object(c);
            // private String threadName;
            thread_info->set_field_value<jref>("threadName", "Ljava/lang/String;", name);
            // private long threadId;
            thread_info->set_field_value<jlong>("threadId", tid);

            // todo ThreadInfo 的其他属性

            thread_infos[count++] = thread_info;
        }
    }

    jarrRef result = Allocator::array(c->loader, "[Ljava/lang/management/ThreadInfo;", count);
    for (int i = 0; i < count; i++) {
        result->setRefElt(i, thread_infos[i]);
    }

    delete[] thread_infos;
    return (jobjectArray) result;
}

void JNICALL Jmm_SetGCNotificationEnabled(JNIEnv *env, jobject mgr, jboolean enabled) {
    unimplemented
}

jobjectArray JNICALL Jmm_GetDiagnosticCommands(JNIEnv *env) {
    unimplemented
}

void JNICALL Jmm_GetDiagnosticCommandInfo(JNIEnv *env, jobjectArray cmds, dcmdInfo *infoArray) {
    unimplemented
}

void JNICALL Jmm_GetDiagnosticCommandArgumentsInfo(JNIEnv *env, jstring commandName, dcmdArgInfo *infoArray) {
    unimplemented
}

jstring JNICALL Jmm_ExecuteDiagnosticCommand(JNIEnv *env, jstring command) {
    unimplemented
}

void JNICALL Jmm_SetDiagnosticFrameworkNotificationEnabled (JNIEnv *env, jboolean enabled) {
    unimplemented
}

static JmmInterface jmm_interface = {
    .reserved1 = nullptr,
    .reserved2 = nullptr,

    .GetVersion = Jmm_GetVersion,
    .GetOptionalSupport = Jmm_GetOptionalSupport,
    .GetThreadInfo = Jmm_GetThreadInfo,

    .GetMemoryPools = Jmm_GetMemoryPools,
    .GetMemoryManagers = Jmm_GetMemoryManagers,
    .GetMemoryPoolUsage = Jmm_GetMemoryPoolUsage,
    .GetPeakMemoryPoolUsage = Jmm_GetPeakMemoryPoolUsage,
    .GetOneThreadAllocatedMemory = Jmm_GetOneThreadAllocatedMemory,
    .GetThreadAllocatedMemory = Jmm_GetThreadAllocatedMemory,
    .GetMemoryUsage = Jmm_GetMemoryUsage,

    .GetLongAttribute = Jmm_GetLongAttribute,
    .GetBoolAttribute = Jmm_GetBoolAttribute,
    .SetBoolAttribute = Jmm_SetBoolAttribute,
    .GetLongAttributes = Jmm_GetLongAttributes,

    .FindCircularBlockedThreads = Jmm_FindCircularBlockedThreads,

    // Not used in JDK 6 or JDK 7
    .GetThreadCpuTime = Jmm_GetThreadCpuTime,

    .GetVMGlobalNames = Jmm_GetVMGlobalNames,
    .GetVMGlobals = Jmm_GetVMGlobals,

    .GetInternalThreadTimes = Jmm_GetInternalThreadTimes,
    .ResetStatistic = Jmm_ResetStatistic,
    .SetPoolSensor = Jmm_SetPoolSensor,
    .SetPoolThreshold = Jmm_SetPoolThreshold,
    .GetPoolCollectionUsage = Jmm_GetPoolCollectionUsage,
    .GetGCExtAttributeInfo = Jmm_GetGCExtAttributeInfo,
    .GetLastGCStat = Jmm_GetLastGCStat,

    .GetThreadCpuTimeWithKind = Jmm_GetThreadCpuTimeWithKind,
    .GetThreadCpuTimesWithKind = Jmm_GetThreadCpuTimesWithKind,

    .DumpHeap0 = Jmm_DumpHeap0,
    .SetVMGlobal = Jmm_SetVMGlobal,
    .reserved6 = nullptr,
    .DumpThreads = Jmm_DumpThreads,
    .SetGCNotificationEnabled = Jmm_SetGCNotificationEnabled,
    .GetDiagnosticCommands = Jmm_GetDiagnosticCommands,
    .GetDiagnosticCommandInfo = Jmm_GetDiagnosticCommandInfo,
    .GetDiagnosticCommandArgumentsInfo = Jmm_GetDiagnosticCommandArgumentsInfo,
    .ExecuteDiagnosticCommand = Jmm_ExecuteDiagnosticCommand,
    .SetDiagnosticFrameworkNotificationEnabled = Jmm_SetDiagnosticFrameworkNotificationEnabled,
};

void *getJmmInterface(jint version) {
    if (version == Jmm_GetVersion(nullptr)) {
        return (void *) &jmm_interface;
    }
    return nullptr;
}