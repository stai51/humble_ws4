// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from custom_message:msg/String.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__STRING__BUILDER_HPP_
#define CUSTOM_MESSAGE__MSG__DETAIL__STRING__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "custom_message/msg/detail/string__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace custom_message
{

namespace msg
{

namespace builder
{

class Init_String_data
{
public:
  Init_String_data()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::custom_message::msg::String data(::custom_message::msg::String::_data_type arg)
  {
    msg_.data = std::move(arg);
    return std::move(msg_);
  }

private:
  ::custom_message::msg::String msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::custom_message::msg::String>()
{
  return custom_message::msg::builder::Init_String_data();
}

}  // namespace custom_message

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__STRING__BUILDER_HPP_
