// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from custom_message:msg/ImuMsg.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__STRUCT_HPP_
#define CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.hpp"
// Member 'first_string'
// Member 'second_string'
#include "std_msgs/msg/detail/string__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__custom_message__msg__ImuMsg __attribute__((deprecated))
#else
# define DEPRECATED__custom_message__msg__ImuMsg __declspec(deprecated)
#endif

namespace custom_message
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ImuMsg_
{
  using Type = ImuMsg_<ContainerAllocator>;

  explicit ImuMsg_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init),
    first_string(_init),
    second_string(_init)
  {
    (void)_init;
  }

  explicit ImuMsg_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    first_string(_alloc, _init),
    second_string(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _first_string_type =
    std_msgs::msg::String_<ContainerAllocator>;
  _first_string_type first_string;
  using _second_string_type =
    std_msgs::msg::String_<ContainerAllocator>;
  _second_string_type second_string;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__first_string(
    const std_msgs::msg::String_<ContainerAllocator> & _arg)
  {
    this->first_string = _arg;
    return *this;
  }
  Type & set__second_string(
    const std_msgs::msg::String_<ContainerAllocator> & _arg)
  {
    this->second_string = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    custom_message::msg::ImuMsg_<ContainerAllocator> *;
  using ConstRawPtr =
    const custom_message::msg::ImuMsg_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<custom_message::msg::ImuMsg_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<custom_message::msg::ImuMsg_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      custom_message::msg::ImuMsg_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<custom_message::msg::ImuMsg_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      custom_message::msg::ImuMsg_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<custom_message::msg::ImuMsg_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<custom_message::msg::ImuMsg_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<custom_message::msg::ImuMsg_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__custom_message__msg__ImuMsg
    std::shared_ptr<custom_message::msg::ImuMsg_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__custom_message__msg__ImuMsg
    std::shared_ptr<custom_message::msg::ImuMsg_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ImuMsg_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->first_string != other.first_string) {
      return false;
    }
    if (this->second_string != other.second_string) {
      return false;
    }
    return true;
  }
  bool operator!=(const ImuMsg_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ImuMsg_

// alias to use template instance with default allocator
using ImuMsg =
  custom_message::msg::ImuMsg_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace custom_message

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__STRUCT_HPP_
