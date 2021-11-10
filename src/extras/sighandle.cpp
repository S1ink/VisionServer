#include "sighandle.h"

static olstream sigout; //this has to be outside of the class
//SigHandle& sig_handle = SigHandle::get();

const std::array<const char*, 31> SigHandle::sigmap = {
    "console hangup", "keyboard interrupt", "quit request",
    "illegal CPU instruction", "trace request", "abort",
    "invalid bus address", "floating point arithmetic error, likely div/0",
    "program killed (unblockable)", "custom signal, not from os",
    "segmentation violation - check for bad '*/&'", "custom signal, not from os",
    "broken pipe", "alarm", "request to terminate", "stack fault",
    "child status has changed", "command to continue",
    "program stopped (unblockable)", "keyboard stop", "backround terminal read",
    "backround terminal write", "urgent socket condition", "CPU limit exceeded",
    "file size limit exceeded", "virtual alarm", "profiling alarm",
    "window size change", "IO now possible", "power restart (terminate)", "bad system call",
};
const std::initializer_list<SigHandle::Sigs> SigHandle::def_ign = {
    Sigs::HUP, Sigs::TRAP, Sigs::USR1, Sigs::USR2, Sigs::PIPE, Sigs::ALRM, 
    Sigs::CHLD, Sigs::CONT, Sigs::STP, Sigs::TTIN, Sigs::TTOU, Sigs::URG, 
    Sigs::VTALRM, Sigs::PROF, Sigs::WINCH, Sigs::IOA, Sigs::SYSC
};
const std::initializer_list<SigHandle::Sigs> SigHandle::def_ex = {
    Sigs::INT, Sigs::QUIT, Sigs::ILL, Sigs::ABRT, Sigs::BUS, Sigs::FPE, 
    Sigs::SEGV, Sigs::TERM, Sigs::STKFLT, Sigs::XCPU, Sigs::XFSZ, Sigs::PWR
};

SigHandle::SigHandle() :
    ign_sigs(def_ign),
    exit_sigs(def_ex)
{
    sigSet();
    setLog(std::ios::app);
}

void SigHandle::sigSet(__sighandler_t exit, __sighandler_t ignore) {
#ifdef __linux__
    struct sigaction action_ignore, action_exit;
    sigemptyset(&(action_ignore.sa_mask));
    sigemptyset(&(action_exit.sa_mask));
    action_ignore.sa_flags = action_exit.sa_flags = 0;
    action_ignore.sa_handler = ignore;
    action_exit.sa_handler = exit;
    for (uint i = 0; i < ign_sigs.size(); i++) {
        sigaction((uint)ign_sigs[i], &action_ignore, NULL);
    }
    for (uint i = 0; i < exit_sigs.size(); i++) {
        sigaction((uint)exit_sigs[i], &action_exit, NULL);
    }
#else
    for (uint i = 0; i < ign_sigs.size(); i++) {
        signal((uint)ign_sigs[i], ignore);
    }
    for (uint i = 0; i < exit_sigs.size(); i++) {
        signal((uint)exit_sigs[i], exit);
    }
#endif
}
#ifdef __linux__
void SigHandle::sigSet(__sigaction_t exit, __sigaction_t ignore) {
    struct sigaction action_ignore, action_exit;
    sigemptyset(&(action_ignore.sa_mask));
    sigemptyset(&(action_exit.sa_mask));
    action_ignore.sa_flags = action_exit.sa_flags = SA_SIGINFO;
    action_ignore.sa_sigaction = ignore;
    action_exit.sa_sigaction = exit;
    for (uint i = 0; i < ign_sigs.size(); i++) {
        sigaction((uint)ign_sigs[i], &action_ignore, NULL);
    }
    for (uint i = 0; i < exit_sigs.size(); i++) {
        sigaction((uint)exit_sigs[i], &action_exit, NULL);
    }
}
void SigHandle::sigSet(bool adv) {
    struct sigaction action_ignore, action_exit;    
    sigemptyset(&(action_ignore.sa_mask));
    sigemptyset(&(action_exit.sa_mask));
    if (adv) {
        action_ignore.sa_flags = action_exit.sa_flags = SA_SIGINFO;
        action_ignore.sa_sigaction = advIgnore;
        action_exit.sa_sigaction = advExit;
    }
    else {
        action_ignore.sa_flags = action_exit.sa_flags = 0;
        action_ignore.sa_handler = sigIgnore;
        action_exit.sa_handler = sigExit;
    }
    for (uint i = 0; i < ign_sigs.size(); i++) {
        sigaction((uint)ign_sigs[i], &action_ignore, NULL);
    }
    for (uint i = 0; i < exit_sigs.size(); i++) {
        sigaction((uint)exit_sigs[i], &action_exit, NULL);
    }
}
#else
void SigHandle::sigSet() {
    for (uint i = 0; i < ign_sigs.size(); i++) {
        signal((uint)ign_sigs[i], sigIgnore);
    }
    for (uint i = 0; i < exit_sigs.size(); i++) {
        signal((uint)exit_sigs[i], sigExit);
    }
}
#endif

SigHandle& SigHandle::get() {
    static SigHandle global;
    return global;
}

void SigHandle::printIgnore(int signum) {
    sigout << dateStamp() << " : Signal(" << signum << ") caught: {" << sigmap[signum - 1] << "} -> ignored\n";
}
void SigHandle::printExit(int signum) {
    sigout << dateStamp() << " : Signal(" << signum << ") caught: {" << sigmap[signum - 1] << "} -> calling exit\n";
}
void SigHandle::sigIgnore(int signum) {
    printIgnore(signum);
}
void SigHandle::sigExit(int signum) {
    printExit(signum);
    exit(signum);
}

#ifdef __linux__
void SigHandle::printInfo(siginfo_t* data) {
    sigout << "~ADVANCED STATISTICS~\n"
        << "errno: " << data->si_errno
        << "\ncode: " << data->si_code
        //<< "\ntrapno: " << data->si_trapno 
        << "\npid: " << data->si_pid
        << "\nuid: " << data->si_uid
        << "\nstatus: " << data->si_status
        << "\nutime: " << data->si_utime
        << "\nstime: " << data->si_stime
        //<< "\nvalue: " << data->si_value
        << "\nint: " << data->si_int
        << "\nptr: " << data->si_ptr
        << "\noverrun: " << data->si_overrun
        << "\ntimerid: " << data->si_timerid
        << "\naddr: " << data->si_addr
        << "\nband: " << data->si_band
        << "\nfd: " << data->si_fd
        << "\naddr_lsb: " << data->si_addr_lsb
        // << "\nlower: " << data->si_lower
        // << "\nupper: " << data->si_upper
        // << "\npkey: " << data->si_pkey
        << "\ncall_addr: " << data->si_call_addr
        << "\nsyscall: " << data->si_syscall
        << "\narch: " << data->si_arch << "\n\n";
}
void SigHandle::advIgnore(int signum, siginfo_t* data, void* ext) {
    printIgnore(signum);
    printInfo(data);
}
void SigHandle::advExit(int signum, siginfo_t* data, void* ext) {
    printExit(signum);
    printInfo(data);
    exit(signum);
}
#endif

void SigHandle::setLog(const std::ios::openmode modes) {
    sigout.setModes(modes);
}
void SigHandle::setLog(const char* file) {
    sigout.setStream(file);
}
void SigHandle::setLog(const char* file, const std::ios::openmode modes) {
    sigout.setStream(file, modes);
}
void SigHandle::setLog(std::ostream* stream) {
    sigout.setStream(stream);
}
void SigHandle::setLog(olstream&& target) {
    sigout = std::move(target);
}
void SigHandle::setLog(const olstream& target) {
    sigout = target;
}
void SigHandle::reset(std::initializer_list<Sigs> exit, std::initializer_list<Sigs> ignore) {
    this->exit_sigs = exit;
    this->ign_sigs = ignore;
    sigSet();
}
void SigHandle::setup(__sighandler_t exit, __sighandler_t ignore) {
    sigSet(exit, ignore);
}
void SigHandle::setup(__sighandler_t exit, std::initializer_list<Sigs> exl, std::initializer_list<Sigs> ignl, __sighandler_t ignore) {
    this->exit_sigs = exl;
    this->ign_sigs = ignl;
    sigSet(exit, ignore);
}
#ifdef __linux__
void SigHandle::setup(__sigaction_t exit, __sigaction_t ignore) {
    sigSet(exit, ignore);
}
void SigHandle::setup(__sigaction_t exit, std::initializer_list<Sigs> exl, std::initializer_list<Sigs> ignl, __sigaction_t ignore) {
    this->exit_sigs = exl;
    this->ign_sigs = ignl;
    sigSet(exit, ignore);
}
void SigHandle::setadv() {
    sigSet(1);
}
#endif