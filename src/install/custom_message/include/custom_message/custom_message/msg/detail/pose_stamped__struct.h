// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from custom_message:msg/PoseStamped.idl
// generated code does not contain a copyright notice

#ifndef CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__STRUCT_H_
#define CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__STRUCT_H_

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
// Member 'pose'
#include "geometry_msgs/msg/detail/pose__struct.h"

/// Struct defined in msg/PoseStamped in the package custom_message.
typedef struct custom_message__msg__PoseStamped
{
  std_msgs__msg__Header header;
  geometry_msgs__msg__Pose pose;
} custom_message__msg__PoseStamped;

// Struct for a sequence of custom_message__msg__PoseStamped.
typedef struct custom_message__msg__PoseStamped__Sequence
{
  custom_message__msg__PoseStamped * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} custom_message__msg__PoseStamped__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // CUSTOM_MESSAGE__MSG__DETAIL__POSE_STAMPED__STRUCT_H_
