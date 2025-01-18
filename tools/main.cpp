#include <iostream>
#include <editline/readline.h>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <string>

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

    // String Handling
    // split, read delimited text from the string we give it
    std::vector<std::string> split(std::string_view str, char delimiter)
    {
        std::vector<std::string> out{};
        std::stringstream ss {std::string{str}};
        std::string item;
        
        //read a block of text from the given stream into item until it hits a delimiter
        while(std::getline(ss, item, delimiter))
        {
            //collects all of these blocks into a std::vector and return it
            out.push_back(item);
        }
        return out;
    }

    //returns and indication of whether a string is either equal to or a prefix of antoher string
    bool is_prefix(std::string_view str, std::string_view of)
    {
        if(str.size() > of.size()) return false;
        return std::equal(str.begin(),str.end(),of.begin());
    }

    // Process Manipulation Tasks
    // use PTRACE_CONT to make the inferior process continue
    void resume(pid_t pid)
    {
        if(ptrace(PTRACE_CONT,pid,nullptr,nullptr) <0 )
        {
            std::cerr << "Couldn't continue\n";
            std::exit(-1);
        }
    }
    
    //wraps a call to waitpid 
    void wait_on_signal(pid_t pid)
    {
        int wait_status;
        int options = 0;
        if(waitpid(pid, &wait_status, options) < 0)
        {
            std::perror("waitpid failed");
            std::exit(-1);
        }
    }

    void handle_command(pid_t pid, std::string_view line)
    {
        auto args = split(line, ' ');
        auto command = args[0];
        if(is_prefix(command, "continue"))
        {
            resume(pid);
            wait_on_signal(pid);
        }
        else
        {
            std::cerr << "Unknown command\n";
        }
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

    // Implement a simple command line interface to interact with the program
    // the user pass commands throught this interface to the debugger line by line
    // when a Ctrl-D <EOF> is passed the line gets a null and the interface closes
    char* line = nullptr;
    while ((line = readline("sdb> ")) != nullptr)
    {
        std::string line_str;

        //If line is empty, the line is freed and the last command passed is recalled
        //using functions from the libedit, history_list and history_length
        if(line == std::string_view(""))
        {
            free(line);
            if(history_length > 0)
            {
                line_str = history_list()[history_length - 1]->line;
            }
        }
        // If line passed by the user is not empty we save it to string
        // add it to the history and free its memory
        else
        {
            line_str = line;
            add_history(line);
            free(line);
        }
        //handle the command on the saved string
        if(!line_str.empty())
        {
            handle_command(pid, line_str);
        }
    }
}
