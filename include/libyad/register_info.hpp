#ifndef YAD_REGISTER_INFO_HPP
#define YAD_REGISTER_INFO_HPP

#include <cstdint>
#include <cstddef>
#include <string_view>
#include <sys/user.h>

namespace yad
{
  /*  we want data about the 124 registers in two separate places
        register_id
        g_register_infos
  */

  //give each register in the system its own unique enumerator value
  enum class register_id{};

  //whether a given register is a GPR, a subregister of GPR(eax =/= rax), FPR or debug register
  enum class register_type
  {
    gpr, sub_gpr, fpr, dr
  };

  //enumarte different ways of interpreting a register
  enum class register_format
  {
    uint, double_float, long_double, vector
  };

  // collects all info we need about a single register
  struct register_info
  {
    register_id id;
    std::string_view name;
    std::int32_t dwarf_id;
    std::size_t size;
    std::size_t offset;
    register_type type;
    register_format format;
  };

  //global array of the informationf for every register in the system
  //inline keyword lets us define this array in the header so we can deduce
  //the number of registers automatically from the initializer
  inline constexpr const register_info g_register_infos[] = {};
}

#endif
