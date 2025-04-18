// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from custom_message:msg/PoseStamped.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__TRAITS_HPP_
#define CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "custom_message/msg/detail/pose_stamped__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__traits.hpp"

namespace custom_message
{

namespace msg
{

inline void to_flow_style_yaml(
  const PoseStamped & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: pose
  {
    out << "pose: ";
    to_flow_style_yaml(msg.pose, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const PoseStamped & msg,
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

  // member: pose
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "pose:\n";
    to_block_style_yaml(msg.pose, out, indentation + 2);
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const PoseStamped & msg, bool use_flow_style = false)
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
  const custom_message::msg::PoseStamped & msg,
  std::ostream & out, size_t indentation = 0)
{
  custom_message::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use custom_message::msg::to_yaml() instead")]]
inline std::string to_yaml(const custom_message::msg::PoseStamped & msg)
{
  return custom_message::msg::to_yaml(msg);
}

template<>
inline const char * data_type<custom_message::msg::PoseStamped>()
{
  return "custom_message::msg::PoseStamped";
}

template<>
inline const char * name<custom_message::msg::PoseStamped>()
{
  return "custom_message/msg/PoseStamped";
}

template<>
struct has_fixed_size<custom_message::msg::PoseStamped>
  : std::integral_constant<bool, has_fixed_size<geometry_msgs::msg::Pose>::value && has_fixed_size<std_msgs::msg::Header>::value> {};

template<>
struct has_bounded_size<custom_message::msg::PoseStamped>
  : std::integral_constant<bool, has_bounded_size<geometry_msgs::msg::Pose>::value && has_bounded_size<std_msgs::msg::Header>::value> {};

template<>
struct is_message<custom_message::msg::PoseStamped>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__TRAITS_HPP_
