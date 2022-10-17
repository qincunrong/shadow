/*
 * The Shadow Simulator
 * See LICENSE for licensing information
 */

#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/ucontext.h>
#include <time.h>

#include "lib/logger/logger.h"
#include "lib/shim/shim.h"
#include "lib/shim/shim_logger.h"
#include "lib/shim/shim_signals.h"
#include "lib/shim/shim_sys.h"
#include "lib/shim/shim_syscall.h"
#include "lib/shim/shim_tls.h"
#include "lib/tsc/tsc.h"

static uint64_t _shim_rdtsc_nanos(bool allowNative) {
    if (allowNative) {
        // To handle this correctly, we'd need to execute a real rdtsc
        // instruction. To do that, we'd need to temporarily allow
        // native rdtsc.  I *think* it's not that hard to implement, but
        // could add a bit of overhead.  Better to notice if we're using
        // rdtsc and avoid it if possible.
        panic("Native rdtsc unimplemented.")
    }
    struct timespec t = {0};
    // *don't* directly call shim_sys_get_simtime_nanos() here.  We need to go
    // through the syscall code to correctly handle the case where
    // `model_unblocked_syscall_latency` is enabled.
    long rv = shim_emulated_syscall(SYS_clock_gettime, CLOCK_REALTIME, &t);
    if (rv != 0) {
        panic("emulated SYS_clock_gettime: %s", strerror(-rv));
    }
    return (uint64_t)t.tv_nsec + (uint64_t)t.tv_sec * 1000000000;
}

static void _shim_rdtsc_handle_sigsegv(int sig, siginfo_t* info, void* voidUcontext) {
    bool oldNativeSyscallFlag = shim_swapAllowNativeSyscalls(true);
    trace("Trapped sigsegv");
    static bool tsc_initd = false;
    static Tsc tsc;
    if (!tsc_initd) {
        trace("Initializing tsc");
        uint64_t hz;
        // TODO: We should move this to host shared memory and avoid parsing
        // here.
        if (sscanf(getenv("SHADOW_TSC_HZ"), "%" PRIu64, &hz) != 1) {
            panic("Couldn't parse SHADOW_TSC_HZ %s", getenv("SHADOW_TSC_HZ"));
        }
        tsc = Tsc_create(hz);
        tsc_initd = true;
    }

    bool handled = false;

    if (info->si_code == SI_KERNEL) {
        // SIGSEGVs generated by executing an rdtsc or rdstscp
        // instruction after disabling them with `prctl` has a code of SI_KERNEL.
        // While this doesn't appear to be specifically documented, it's really the
        // only code documented in `sigaction(2)` that would make sense. e.g.,
        // a SIGSEGV caused by accessing an unmapped region would have code
        // SEGV_MAPERR.
        //
        // Explicitly performing this check helps prevent accessing a potentially
        // inaccessible pointer below to read the program counter.
        ucontext_t* ctx = (ucontext_t*)(voidUcontext);
        greg_t* regs = ctx->uc_mcontext.gregs;
        unsigned char* insn = (unsigned char*)regs[REG_RIP];
        if (isRdtsc(insn)) {
            trace("Emulating rdtsc");
            uint64_t nanos = _shim_rdtsc_nanos(oldNativeSyscallFlag);
            uint64_t rax, rdx;
            uint64_t rip = regs[REG_RIP];
            Tsc_emulateRdtsc(&tsc, &rax, &rdx, &rip, nanos);
            regs[REG_RDX] = rdx;
            regs[REG_RAX] = rax;
            regs[REG_RIP] = rip;
            handled = true;
        } else if (isRdtscp(insn)) {
            trace("Emulating rdtscp");
            uint64_t nanos = _shim_rdtsc_nanos(oldNativeSyscallFlag);
            uint64_t rax, rdx, rcx;
            uint64_t rip = regs[REG_RIP];
            Tsc_emulateRdtscp(&tsc, &rax, &rdx, &rcx, &rip, nanos);
            regs[REG_RDX] = rdx;
            regs[REG_RAX] = rax;
            regs[REG_RCX] = rcx;
            regs[REG_RIP] = rip;
            handled = true;
        }
    }

    // Restore the old native-syscalls disposition *before* potentially
    // delegating to generic handler below, so that it can recognize the whether
    // SIGSEGV was raised from managed code or shim code.
    shim_swapAllowNativeSyscalls(oldNativeSyscallFlag);

    if (!handled) {
        trace("SIGSEGV not recognized as rdtsc; handling as error");
        shim_handle_hardware_error_signal(SIGSEGV, info, voidUcontext);
    }
}

void shim_rdtsc_init() {
    // Force a SEGV on any rdtsc or rdtscp instruction.
    if (prctl(PR_SET_TSC, PR_TSC_SIGSEGV) < 0) {
        panic("pctl: %s", strerror(errno));
    }

    // Install our own handler to emulate.
    if (sigaction(SIGSEGV,
                  &(struct sigaction){
                      .sa_sigaction = _shim_rdtsc_handle_sigsegv,
                      // SA_NODEFER: Handle recursive SIGSEGVs, so that it can
                      // "rethrow" the SIGSEGV and in case of a bug in the
                      // handler.
                      // SA_SIGINFO: Required because we're specifying
                      // sa_sigaction.
                      // SA_ONSTACK: Use the alternate signal handling stack, to avoid interfering
                      // with userspace thread stacks.
                      .sa_flags = SA_SIGINFO | SA_NODEFER | SA_ONSTACK,
                  },
                  NULL) < 0) {
        panic("sigaction: %s", strerror(errno));
    }
}
