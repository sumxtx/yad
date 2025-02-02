#include <libyad/process.hpp>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

std::unique_ptr<yad::process> yad::process::launch(std::filesystem::path path)
{
  pid_t pid;
  if((pid = fork()) < 0)
  {
    error::send_errno("fork failed");
  }

  if(pid == 0)
  {
    if(ptrace(PTRACE_TRACEME,0, nullptr, nullptr) < 0)
    {
      error::send_errno("Tracing failed");
    }
    if(execlp(path.c_str(), path.c_str(), nullptr) < 0)
    {
      error::send_errno("exec failed");
    }
  }
  std::unique_ptr<process> proc (new process(pid, /*terminate on end*/true));
  proc->wait_on_signal();

  return proc;
}

std::unique_ptr<yad::process> yad::process::attach(pid_t pid)
{
  if(pid == 0)
  {
    error::send_errno("Invalid PID");
  }
  if(ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0)
  {
    error::send_errno("Could not attach");
  }

  std::unique_ptr<process> proc(new process(pid, /*terminate_on_end*/false));
  proc->wait_on_signal;

  return proc;
}

yad::process::~process()
{
  //if we have a valid PID when the destructor runs, then we want to dettach
  //for PTRACE_DETACH to work the inferior process must be stopped
  //so if it is currently running we send it a SIGSTOP and wait for it to stop
  if(pid_ != 0)
  {
    int status;
    if (state_ == process_state::running)
    {
      kill(pid_, SIGSTOP);
      waitpid(pid_, &status, 0);
    }
    //we then detach from the process and let it continue
    ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
    kill(pid_, SIGCONT);

    if(terminate_on_end_)
    {
      kill(pid_, SIGKILL);
      waitpid(pid_, &status, 0);
    }
  }
}

void yad::process::resume()
{
// issue a PTRACE_CONT, check for errors and update the state to running
  if(ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0)
  {
    error::send_errno("Could not resume");
  }
  state_ = process_state::running;
}

yad::stop_reason::stop_reason(int wait_status)
{
  if(WIFEXITED(wait_status))
  {
    reason = process_state::exited;
    info = WEXITSTATUS(wait_status);
  }
  else if(WIFSIGNALES(wait_status))
  {
    reason = process_state::terminated;
    info = WTERMSIG(wait_status);
  }
  else if(WIFSTOPPED(wait_status))
  {
    reason = process_state::stopped;
    info = WSTOPSIG(wait_status);
  }
}

yad::stop_reason yad::process::wait_on_signal()
{
  int wait_status;
  int options = 0;
  if(waitpid(pid_,&wait_status,options) < 0)
  {
    error::send_errno("waitpid failed");
  }
  stop_reason reason(wait_status);
  state_ = reason.reason;
  return reason;
}

