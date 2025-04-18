// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from custom_message:msg/ImuMsg.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__TRAITS_HPP_
#define CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "custom_message/msg/detail/imu_msg__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"
// Member 'first_string'
// Member 'second_string'
#include "std_msgs/msg/detail/string__traits.hpp"

namespace custom_message
{

namespace msg
{

inline void to_flow_style_yaml(
  const ImuMsg & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: first_string
  {
    out << "first_string: ";
    to_flow_style_yaml(msg.first_string, out);
    out << ", ";
  }

  // member: second_string
  {
    out << "second_string: ";
    to_flow_style_yaml(msg.second_string, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const ImuMsg & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: first_string
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "first_string:\n";
    to_block_style_yaml(msg.first_string, out, indentation + 2);
  }

  // member: second_string
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "second_string:\n";
    to_block_style_yaml(msg.second_string, out, indentation + 2);
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const ImuMsg & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace custom_message

namespace rosidl_generator_traits
{

[[deprecated("use custom_message::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const custom_message::msg::ImuMsg & msg,
  std::ostream & out, size_t indentation = 0)
{
  custom_message::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use custom_message::msg::to_yaml() instead")]]
inline std::string to_yaml(const custom_message::msg::ImuMsg & msg)
{
  return custom_message::msg::to_yaml(msg);
}

template<>
inline const char * data_type<custom_message::msg::ImuMsg>()
{
  return "custom_message::msg::ImuMsg";
}

template<>
inline const char * name<custom_message::msg::ImuMsg>()
{
  return "custom_message/msg/ImuMsg";
}

template<>
struct has_fixed_size<custom_message::msg::ImuMsg>
  : std::integral_constant<bool, has_fixed_size<std_msgs::msg::Header>::value && has_fixed_size<std_msgs::msg::String>::value> {};

template<>
struct has_bounded_size<custom_message::msg::ImuMsg>
  : std::integral_constant<bool, has_bounded_size<std_msgs::msg::Header>::value && has_bounded_size<std_msgs::msg::String>::value> {};

template<>
struct is_message<custom_message::msg::ImuMsg>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__TRAITS_HPP_
