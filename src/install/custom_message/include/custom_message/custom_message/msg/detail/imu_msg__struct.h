// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from custom_message:msg/ImuMsg.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__STRUCT_H_
#define CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'first_string'
// Member 'second_string'
#include "std_msgs/msg/detail/string__struct.h"

/// Struct defined in msg/ImuMsg in the package custom_message.
typedef struct custom_message__msg__ImuMsg
{
  std_msgs__msg__Header header;
  std_msgs__msg__String first_string;
  std_msgs__msg__String second_string;
} custom_message__msg__ImuMsg;

// Struct for a sequence of custom_message__msg__ImuMsg.
typedef struct custom_message__msg__ImuMsg__Sequence
{
  custom_message__msg__ImuMsg * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_message__msg__ImuMsg__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__IMU_MSG__STRUCT_H_
