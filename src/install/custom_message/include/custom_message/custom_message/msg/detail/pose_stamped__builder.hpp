// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_message:msg/PoseStamped.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__BUILDER_HPP_
#define CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_message/msg/detail/pose_stamped__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_message
{

namespace msg
{

namespace builder
{

class Init_PoseStamped_pose
{
public:
  explicit Init_PoseStamped_pose(::custom_message::msg::PoseStamped & msg)
  : msg_(msg)
  {}
  ::custom_message::msg::PoseStamped pose(::custom_message::msg::PoseStamped::_pose_type arg)
  {
    msg_.pose = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_message::msg::PoseStamped msg_;
};

class Init_PoseStamped_header
{
public:
  Init_PoseStamped_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PoseStamped_pose header(::custom_message::msg::PoseStamped::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_PoseStamped_pose(msg_);
  }

private:
  ::custom_message::msg::PoseStamped msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_message::msg::PoseStamped>()
{
  return custom_message::msg::builder::Init_PoseStamped_header();
}

}  // namespace custom_message

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__BUILDER_HPP_
