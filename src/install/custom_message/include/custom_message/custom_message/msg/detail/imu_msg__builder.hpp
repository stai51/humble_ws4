// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_message:msg/ImuMsg.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__BUILDER_HPP_
#define CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_message/msg/detail/imu_msg__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_message
{

namespace msg
{

namespace builder
{

class Init_ImuMsg_second_string
{
public:
  explicit Init_ImuMsg_second_string(::custom_message::msg::ImuMsg & msg)
  : msg_(msg)
  {}
  ::custom_message::msg::ImuMsg second_string(::custom_message::msg::ImuMsg::_second_string_type arg)
  {
    msg_.second_string = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_message::msg::ImuMsg msg_;
};

class Init_ImuMsg_first_string
{
public:
  explicit Init_ImuMsg_first_string(::custom_message::msg::ImuMsg & msg)
  : msg_(msg)
  {}
  Init_ImuMsg_second_string first_string(::custom_message::msg::ImuMsg::_first_string_type arg)
  {
    msg_.first_string = std::move(arg);
    return Init_ImuMsg_second_string(msg_);
  }

private:
  ::custom_message::msg::ImuMsg msg_;
};

class Init_ImuMsg_header
{
public:
  Init_ImuMsg_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ImuMsg_first_string header(::custom_message::msg::ImuMsg::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_ImuMsg_first_string(msg_);
  }

private:
  ::custom_message::msg::ImuMsg msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_message::msg::ImuMsg>()
{
  return custom_message::msg::builder::Init_ImuMsg_header();
}

}  // namespace custom_message

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__BUILDER_HPP_
