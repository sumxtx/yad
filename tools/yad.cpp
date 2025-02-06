/*
 *  Usage:
 *          yad <program name>
 *          yad -p <pid>
 */
#include <iostream>
#include <unistd.h>
#include <string_view>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <editline/readline.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include <libyad/process.hpp>
#include <libyad/error.hpp>


//Anonymous launch space if they are used only in the implementation file they belong to
namespace
{
  /* Launches, Attaches to the given program name or PID, return the PID of the inferior
     A process can initiate a trace by calling fork(2) and having the resulting child do a
     PTRACE_TRACEME, followed (typically) by an execve(2).  Alternatively, one process may
     commence tracing another process using PTRACE_ATTACH or PTRACE_SEIZE. */
  std::unique_ptr<yad::process> attach(int argc, const char **argv)
  {
    //Passing PID
    if(argc == 3 && argv[1] == std::string_view("-p"))
    {
      //pid is equal the second argument
      pid_t pid = std::atoi(argv[2]);
      return yad::process::attach(pid);
    }
    // Passing process name
    else
    {
      const char* program_path = argv[1];
      //Try to fork, and return if failes
      return yad::process::launch(program_path);
    }
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
  void print_stop_reason(const yad::process& process, yad::stop_reason reason)
  {
    std::cout << "Process " << process.pid() << ' ' ;
    switch (reason.reason)
    {
      case yad::process_state::exited:
        std::cout << "exited with status " << static_cast<int>(reason.info);
        break;
      case yad::process_state::terminated:
        std::cout << "terminated with signal " << sigabbrev_np(reason.info);
        break;
      case yad::process_state::stopped:
        std::cout << "stopped  with signal " << sigabbrev_np(reason.info);
        break;
    }
    std::cout << std::endl;
  }

  void handle_command(std::unique_ptr<yad::process>& process, std::string_view line)
  {
    auto args = split(line, ' ');
    auto command = args[0];
    if(is_prefix(command, "continue"))
    {
      process->resume();
      auto reason = process->wait_on_signal();
      print_stop_reason(*process, reason);
    }
    else
    {
      std::cerr << "Unknown command\n";
    }
  }
}

void main_loop(std::unique_ptr<yad::process>& process)
{
  // Implement a simple command line interface to interact with the program
  // the user pass commands throught this interface to the debugger line by line
  // when a Ctrl-D <EOF> is passed the line gets a null and the interface closes
  char* line = nullptr;
  while ((line = readline("yad> ")) != nullptr)
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
      try
      {
        handle_command(process, line_str);
      }
      catch(const yad::error& err)
      {
        std::cout << err.what() << '\n';
      }
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

  try 
  {
    auto process = attach(argc, argv);
    main_loop(process);
  }
  catch(const yad::error& err)
  {
    std::cout << err.what() << '\n';
  }
}
