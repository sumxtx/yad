#ifndef YAD_PROCESS_HPP
#define YAD_PROCESS_HPP

#include <filesystem>
#include <memory>
#include <sys/types.h>
#include <cstdint>

namespace yad
{
  // Keep track of the current running state of the process
  enum class process_state
  {
    stopped,
    running,
    exited,
    terminated
  };
  //Type for the wait_on_signal return
  //holds the reason for a stop, the process exited, terminated or just stopped

  struct stop_reason
  {
    stop_reason(int wait_status);

    process_state reason;
    //some information about the stop, return value of the exit, or the signal that
    //caused a stop or termination
    std::uint8_t info;
  };

  //We will need a type that represent any running process we can
  //  launch
  //  attach
  //  continue
  //  wait on for signal
  class process
  {
    public:
      ~process();

      // We shouldn't be able to copy and yad::process object, because
      // that would create an entire new process on the system, so
      // Users of the library will need to interact with yad::process through pointers
      // smart pointers will automatically manage the allocated memory
      static std::unique_ptr<process> launch(std::filesystem::path path, bool debug = true);
      static std::unique_ptr<process> attach(pid_t pid);

      void resume();
      stop_reason wait_on_signal();

      // We need to make sure that users cannot construct a process object 
      // or accidentally copy it without
      // going through the static functions above

      // Disable the default constructor
      process() = delete;
      // Disable copy and move behaviour
      process(const process&) = delete;
      process& operator=(const process&) = delete;

      pid_t pid() const {return pid_;}

      // Expose the process state to the user
      process_state state() const { return state_; }

    private:
      // Way for the static members to construct a process object
      // private constructor
      process(pid_t pid, bool terminate_on_end, bool is_attached)
        : pid_(pid), terminate_on_end_(terminate_on_end), is_attached_(is_attached) {}

      pid_t pid_ = 0;
      // We should clean up the inferior process if we launched it ourselves
      // but let it running otherwise
      // so we added the public destructor ~process()
      // and a member totrack wheter we should terminate the process
      bool terminate_on_end_ = true;

      // Member to track the state the process is in
      process_state state_ = process_state::stopped;

      bool is_attached_ = true; //PAGE62
  };
}

#endif
/*  

    C++11 introduced the = delete syntax to disable certain operations for a class, 
    making it a powerful tool for managing resource ownership, 
    enforcing invariants, or preventing unintended behaviors in your class design.

    1. Copy Constructor Deletion (process(const process&) = delete;)

    What is it? The copy constructor is a special constructor invoked 
    when a new object is created as a copy of an existing object. 
    It typically takes the form of:

    process(const process& other);

    When an object of type process is initialized from another process object 
    the copy constructor is invoked to initialize p2 as a copy of p1.

    process p2 = p1;

    What does = delete do? By explicitly deleting the copy constructor 
    you tell the compiler that this constructor is not allowed to exist. 
    Any attempt to create a process object as a copy of another process 
    will result in a compilation error

    process(const process&) = delete;

    Why do this? There are several technical reasons to disable the copy constructor:

    Resource management: If your class manages resources (like dynamically allocated memory, file handles, etc.), copying the object could result in multiple objects owning the same resource, potentially leading to double deletion or other undefined behaviors.
    Unique ownership: If the object represents something with unique ownership (e.g., a system resource), allowing copies could violate the ownership semantics, leading to logical errors.
    Non-copyable types: Some objects, like std::mutex, std::thread, or smart pointers (std::unique_ptr), are explicitly non-copyable because copying them would break their intended semantics (e.g., double locking or sharing ownership).

    2. Copy Assignment Operator Deletion (process& operator=(const process&) = delete;)

    What is it? The copy assignment operator is a special member function 
    used when an existing object is assigned the value of another object of the same type. 
    It has the following form:

    process& operator=(const process& other);

    The copy assignment operator is invoked when an already constructed object 
    is assigned the value of another object (e.g., p2 = p1;).

    What does = delete do? 
    By marking this operator as delete, 
    any attempt to assign one process object to another will be prevented at compile-time, 
    just like with the copy constructor. In practical terms, 
    this means any expression like p2 = p1; 
    where both p2 and p1 are of type process will result in a compile-time error.

*/
