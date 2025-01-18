#include <iostream>
#include <unistd.h>

/*
 *  Usage:
 *          yad <program name>
 *          yad -p <pid>
 *
 *
 */

//Anonymous launch space if they are used only in the implementation file they belong to
namespace
{
    /* Launches, Attaches to the given program name or PID, return the PID of the inferior
    A process can initiate a trace by calling fork(2) and having the resulting child do a
       PTRACE_TRACEME, followed (typically) by an execve(2).  Alternatively, one process may
       commence tracing another process using PTRACE_ATTACH or PTRACE_SEIZE. */
    pid_t attach(int argc, const char **argv)
    {
        pid_t pid = 0;
        //Passing PID
        if(argc == 3 && argv[1] == std::string_view("-p"))
        {
            //pid is equal the second argument
            pid = std::atoi(argv[2]);
            
            //Check if process id passed is correct
            if(pid <= 0)
            {
                std::cerr << "Invalid pid\n";
                return(-1);
            }
            
            //Tries to attach ptrace to the given pid
            if (ptrace(PTRACE_ATTACH, pid, /*addr*/nullptr, /*data*/nullptr) < 0)
            {
                std::perror("Could not attach to process");
                return(-1);
            }
        }
        // Passing process name
        else
        {
            const char* program_path = argv[1];
            //Try to fork, and return if failes
            if((pid = fork()) < 0)
            {
                std::perror("fork failed");
            }

            //If fork succeded and we are in the child process
            if(pid == 0)
            {
                // Execute debuggee in the child process
                // PTRACE_TRACEME allow us to send more ptrace requests to this process in the future
                if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0)
                {
                    std::perror("Tracing failed");
                    return(-1);
                }
                // After enabling process tracing we call exec on the program to be traced
                if (execlp(program_path, program_path, nullptr) < 0)
                {
                    std::perror("Exec failed");
                    return(-1);
                }
            }
        }
        return pid;
    }
}

int main(int argc, const char **argv)
{
    if(argc == 1)
    {
        std::cerr << "No arguments given\n";
        return (-1);
    }

    pid_t pid = attach(argc, argv);

    int wait_status;
    int options = 0;
    //Wait for the process to stop after we hace attached to it
    if (waitpid(pid, &wait_status, options) < 0)
    {
        std::perror("waitpid failed");
    }
}
