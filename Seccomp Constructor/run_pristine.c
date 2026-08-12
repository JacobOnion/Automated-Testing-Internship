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

    printf("\n SYSCALL NUMBERS\n\n");

    FILE* syscallFile = fopen("syscalls.txt", "w");
    for (int i = 0; i < syscallCount; i++) {
        long syscall = syscalls[i];
        fprintf(syscallFile, "%ld\n", syscall);
        //if (strstr(path, "/root")) {
        //printf("SYSCALL NUM: %ld\n", syscall);
        
        const char *name = seccomp_syscall_resolve_num_arch(
            SCMP_ARCH_X86_64,
            syscall
        );

        printf("%s\n", name);

    }
    fclose(syscallFile);
    printf("\n TOTAL UNIQUE SYSCALLS: %d\n\n", syscallCount);
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
        printf("PRISTINE PROGRAM EXITING\n");
        display_paths();
        return 1;
    }

    for (int i = 0; i < processCount; i++) {
        if (processes[i] == pid) {
            processes[i] = processes[--processCount];
            
            //fprintf(stderr, "Removing process %d, processNum is %d\n", pid, processCount);
            
            //if (processCount == 1)
            //    printf("\n\nTRACING FINISHED\n\n");
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
        
        int result = execl("build/Pristine", "Pristine", NULL);

        _exit(result);
    }

    else {
        processes [processCount++] = child;
        int status;
        //fprintf(stderr, "before waitpid\n");
        waitpid(child, &status, 0);
        //fprintf(stderr, "after waitpid\n");


        //Enable syscall tracing
        ptrace(PTRACE_SETOPTIONS, child, 0, PTRACE_O_TRACESYSGOOD |
             PTRACE_O_TRACEEXEC |
              PTRACE_O_TRACEFORK |
               PTRACE_O_TRACEVFORK |
                PTRACE_O_TRACECLONE);
        
        
        ptrace(PTRACE_SYSCALL, child, 0, 0);
        //fprintf(stderr, "after ptrace\n");
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
                        
                        
                        //fprintf(stderr, "NEW PROCESS: %lu\n", new_pid);
                    
                        processes[processCount++] = new_pid;
                        

                        //fprintf(stderr, "Adding process %lu, processNum is %d\n", new_pid, processCount);
                        

                        ptrace(PTRACE_SYSCALL, new_pid, 0, 0);
                    }


                    ptrace(PTRACE_SYSCALL, pid, 0, 0);
                    continue;
                }


                if (sig == (SIGTRAP | 0x80)) {
                    //fprintf(stderr, "entering SIGTRAP\n");
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
                        //fprintf(stderr, "SYSCALL: open/openat() detected: %lu\n", regs.rip);
                        char path[4096];
                        
                        read_child(pid, regs.rsi, path, sizeof(path));
                        //path[4095] = '\0';
                        //pathArray[pathCount++] = strdup(path);
                        
                        /*char *copy = malloc(strlen(path) + 1);
                        strcpy(copy, path);
                        pathArray[pathCount++] = copy;
                        */
                        /*
                        if (pathCount >= 1024) {
                            fprintf(stderr, "pathArray overflow\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }
                        */
                        char *copy = malloc(strlen(path) + 1);
                        if (!copy) {
                            perror("malloc");
                            kill(pid, SIGKILL);
                            return 1;
                        }

                        strcpy(copy, path);
                        //pathArray[pathCount++] = copy;
                        

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


                        //fprintf(stderr, "SYSCALL: open/openat() path: %s\n", path);
                    
                        /*if (strcmp(path, "/autograder/source/tests/fail_dir/bad.txt") == 0) {
                            fprintf(stderr, "ERROR: ACCESSING INVALID DIRECTORY\n");
                            kill(pid, SIGKILL);
                        }*/
                    
                    }
                        
                        // OpenGL uses sockets anyway, so how do I check if they are dangerous?
                        /*if (syscall_num == SYS_socket 
                            && (regs.rdi == AF_INET
                            || regs.rdi == AF_INET6)) {
                            printf("\nERROR: network socket detected\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }*/

                        /*if (syscall_num == SYS_inotify_add_watch) {
                            printf("\nERROR: inotify event detected\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }*/
                        
                        if (syscall_num == SYS_ptrace) {
                            printf("\nERROR: ptrace tampering detected\n\n");
                            kill(pid, SIGKILL);
                            return 1;
                        }

                        

                    //}
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