#ifndef YAD_ERROR_HPP
#define YAD_ERROR_HPP

#include <stdexcept>
#include <cstring>
#include <cerrno>

namespace yad
{
  // Inherits from the std::runtime_error, as it's a special kind of runtime error
  class error : public std::runtime_error
  {
    public:
      // [[noreturn]] - attribute to tell the compiler these functions does note return control flow when it exits
      // prevents unnecessary warnings
      //error:send, takes a message to use as ther error description
      [[noreturn]] static void send(const std::string& what)
      {
        throw error(what);
      }
      //error::send_errno uses the contents of errno as the error description
      //adding the message we provide as prefix
      [[noreturn]] static void send_errno(const std::string& prefix)
      {
        //strerror similar to perror but it returns a string instead of printing to stderr
        throw error(prefix + ": " + std::strerror(errno));
      }
    private:
      error(const std::string& what): std::runtime_error(what){}
  };
}

#endif
