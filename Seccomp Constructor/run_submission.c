#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <seccomp.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <pwd.h>

#define MAX_PROCESSES 256

pid_t processes[256];
int processCount = 0;
pid_t openGL_PID;

char** pathArray = NULL;
size_t pathCount = 0;
size_t capacity = 0;

long syscalls[1024];
int syscallCount;


// Base filter for standard blocked syscalls
void setup_base_filter(void)
{
    scmp_filter_ctx ctx;

    ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx) {
        perror("seccomp_init");
        exit(1);
    }

    // Process inspection / tampering
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(ptrace), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(process_vm_readv), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(process_vm_writev), 0);

    // Kernel introspection
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(perf_event_open), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(bpf), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(userfaultfd), 0);

    // Namespace / mounting
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(mount), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(umount2), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(pivot_root), 0);

    // Reboot / kernel operations
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(reboot), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(kexec_load), 0);

    seccomp_rule_add(ctx,
                 SCMP_ACT_KILL,
                 SCMP_SYS(socket),
                 1,
                 SCMP_A0(SCMP_CMP_EQ, AF_INET));

    seccomp_rule_add(ctx,
                 SCMP_ACT_KILL,
                 SCMP_SYS(socket),
                 1,
                 SCMP_A0(SCMP_CMP_EQ, AF_INET6));
    
    // Inotify (you already dislike it)
    //seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(inotify_init), 0);
    //seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(inotify_init1), 0);
    //seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(inotify_add_watch), 0);

    if (seccomp_load(ctx) < 0) {
        perror("seccomp_load");
        exit(1);
    }

    seccomp_release(ctx);
}

int setup_whitelist() {
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_TRAP);
    if (ctx == NULL)
        return -1;

    // Build seccomp filter
    FILE* syscallFile = fopen("syscalls.txt", "r");
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), syscallFile) != NULL) {
        //printf("syscall num %s", buffer);
        if (seccomp_rule_add(ctx, SCMP_ACT_ALLOW, atoi(buffer), 0) < 0) {
            seccomp_release(ctx);
            return -1;
        }
    }

    if (seccomp_load(ctx) < 0) {
        seccomp_release(ctx);
        return -1;
    }

    seccomp_release(ctx);
    return 0;
}


void display_paths() { // TODO: start checking for WRITES SPECIFICALLY (why would user write to local dirs)
    for (int i = 0; i < pathCount; i++) {
        char* path = pathArray[i];
        if (strstr(path, "/dev/")) {
            printf("PATH: %s\n", path);   
        }
    }

    for (int i = 0; i < pathCount; i++) {
        char* path = pathArray[i];
        if (strstr(path, "/etc/")) {
            printf("PATH: %s\n", path);   
        }
    }

    for (int i = 0; i < pathCount; i++) {
        char* path = pathArray[i];
        if (strstr(path, "/ssh/")) {
            printf("PATH: %s\n", path);   
        }
    }

    for (int i = 0; i < pathCount; i++) {
        char* path = pathArray[i];
        if (strstr(path, "/tmp/")) {
            printf("PATH: %s\n", path);   
        }
    }

    for (int i = 0; i < pathCount; i++) {
        char* path = pathArray[i];
        if (strstr(path, "/root")) {
            printf("PATH: %s\n", path);   
        }
    }

}



void read_child(pid_t child, unsigned long addr, char *buf, size_t max)
{
    size_t i = 0;

    while (i < max - 1) {
        long word = ptrace(PTRACE_PEEKDATA, child, addr + i, NULL);

        for (int j = 0; j < sizeof(long) && i < max - 1; j++) {
            char c = (word >> (j * 8)) & 0xFF;

            buf[i++] = c;
            if (c == '\0') {
                return;
            }
        }
    } 

    buf[max - 1] = '\0';
}

int remove_process(pid_t pid) 
{
    if (pid == openGL_PID)
    {
        printf("OPENGL PROGRAM EXITING\n");
        display_paths();
        return 1;
    }

    for (int i = 0; i < processCount; i++) {
        if (processes[i] == pid) {
            processes[i] = processes[--processCount];
            return 0;
        }
    }
}


int main() {
    pid_t child = fork();
    openGL_PID = child;
    if (child == 0) {
        setvbuf(stdout, NULL, _IONBF, 0);
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        raise(SIGSTOP);
 
        setup_base_filter();
        
        setup_whitelist();

             int result = execl("build/Submission", "Submission", NULL);

        _exit(result);
    }

    else {
        processes [processCount++] = child;
        int status;
        waitpid(child, &status, 0);


        //Enable syscall tracing
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD |
             PTRACE_O_TRACEEXEC |
              PTRACE_O_TRACEFORK |
               PTRACE_O_TRACEVFORK |
                PTRACE_O_TRACECLONE);
        
        
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        int entering = 1;  // Only run syscall tracing on entry, not exit

        while (1) {
            fflush(stdout);

            pid_t pid = waitpid(-1, &status, __WALL);

            if (pid == -1) {
                fprintf(stderr, "WAITPID FAILIURE\n");
                break;
            }

            if (WIFEXITED(status)) {
                int code = WEXITSTATUS(status);
                if (remove_process(pid) == 1)
                    return 1;
                if (processCount <= 0)
                    return 0;
            }
            
            if (WIFSIGNALED(status)) {
                fprintf(stderr, "FAILED THIS PROCESS: killed by %d\n", WTERMSIG(status));
                //ptrace(PTRACE_SYSCALL, child, NULL, 0);
                if (remove_process(pid) == 1)
                    return 1;

                if (processCount <= 0)
                    return 1;

                continue;
            }
            
            if (WIFSTOPPED(status)) {
                int sig = WSTOPSIG(status);
                
                // Syscall handling
                if (sig == SIGTRAP) {

                    char path[128];
                    snprintf(path, sizeof(path), "/proc/%d/comm", pid);

                    FILE *f = fopen(path, "r");
                    if (f) {
                        char name[64];
                        fgets(name, sizeof(name), f);
                        
                        //fprintf(stderr, "PID %d is %s", pid, name);
                        
                        fclose(f);
                    }

                    // Get code from wait status
                    unsigned int event = (unsigned int)status >> 16;

                    if (event == PTRACE_EVENT_CLONE ||
                        event == PTRACE_EVENT_FORK ||
                        event == PTRACE_EVENT_VFORK ||
                        event == PTRACE_EVENT_EXEC) {

                        unsigned long new_pid;
                        ptrace(PTRACE_GETEVENTMSG, pid, 0, &new_pid);
                                            
                        processes[processCount++] = new_pid;
                                                
                        ptrace(PTRACE_SYSCALL, new_pid, 0, 0);
                    }


                    ptrace(PTRACE_SYSCALL, pid, 0, 0);
                    continue;
                }


                // ILLEGAL SYSCALL
                if (sig == SIGSYS) {
                    struct user_regs_struct regs;
                    ptrace(PTRACE_GETREGS, pid, NULL, &regs);

                    fprintf(stderr, "Seccomp violation: syscall %lld\n", regs.orig_rax);
                    
                    for (int i = 0; i < processCount; i++)
                    {
                        kill(processes[i], SIGKILL);
                    }
                    processCount = 0;


                    //ptrace(PTRACE_CONT, pid, NULL, 0);
                }





                if (sig == (SIGTRAP | 0x80)) {
                    struct user_regs_struct regs;
                    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                    long syscall_num = regs.orig_rax;
                    
                    if (syscall_num < 0)
                        goto skip;

                    for (int i = 0; i <= syscallCount; i++)
                    {
                        if (i >= syscallCount) {
                            syscalls[syscallCount++] = syscall_num;
                            break;
                        }

                        if (syscalls[i] == syscall_num)
                            break;
                    }
                    //printf("syscallCount: %d\n", syscallCount);


                    if (syscall_num == SYS_openat || syscall_num == SYS_open) {  // ADD BACK OPEN
                        char path[4096];
                        
                        read_child(pid, regs.rsi, path, sizeof(path));
                        char *copy = malloc(strlen(path) + 1);
                        if (!copy) {
                            perror("malloc");
                            kill(pid, SIGKILL);
                            return 1;
                        }

                        strcpy(copy, path);

                        if (pathCount == capacity) {
                            capacity = (capacity == 0) ? 16 : capacity * 2;
                            
                            char **tmp = realloc(pathArray, capacity * sizeof(*pathArray));
                            if (tmp == NULL) {
                                perror("realloc");
                                exit(EXIT_FAILURE);
                            }

                            pathArray = tmp;
                        
                        }
                        pathArray[pathCount++] = copy;

                        if (strstr(path, "/tests")) {
                            fprintf(stderr, "ERROR: /tests accessed\n\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }

                        if (strstr(path, "/results")) {
                            fprintf(stderr, "ERROR: /results accessed\n\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }

                        if (strstr(path, "run_autograder")) {
                            fprintf(stderr, "ERROR: run_autograder accessed\n\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }

                        if (strstr(path, "premake")) {
                            fprintf(stderr, "ERROR: premake accessed\n\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }
                    
                    }
                        if (syscall_num == SYS_ptrace) {
                            printf("\nERROR: ptrace tampering detected\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }
                    skip:;
                    //entering = !entering;
                    ptrace(PTRACE_SYSCALL, pid, 0, 0);
                }
                else {
                if (sig == SIGSEGV) {
                        fprintf(stderr, "CRASH: Segfault detected! Passing signal through...\n");
                        //ptrace(PTRACE_SYSCALL, child, NULL, sig);
                        ptrace(PTRACE_CONT, pid, NULL, sig);
                    }
                    else {
                        long rc = ptrace(PTRACE_SYSCALL, pid, NULL, sig);
                        //ptrace(PTRACE_CONT, child, NULL, 0);
                    }
                }
            }
        }
    }

    return 0;
}