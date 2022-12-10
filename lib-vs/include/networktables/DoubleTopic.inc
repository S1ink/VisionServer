// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include "networktables/DoubleTopic.h"
#include "networktables/NetworkTableType.h"
#include "ntcore_cpp.h"

namespace nt {

inline DoubleSubscriber::DoubleSubscriber(
    NT_Subscriber handle, double defaultValue)
    : Subscriber{handle},
      m_defaultValue{defaultValue} {}

inline double DoubleSubscriber::Get() const {
  return Get(m_defaultValue);
}

inline double DoubleSubscriber::Get(
    double defaultValue) const {
  return ::nt::GetDouble(m_subHandle, defaultValue);
}

inline TimestampedDouble DoubleSubscriber::GetAtomic() const {
  return GetAtomic(m_defaultValue);
}

inline TimestampedDouble DoubleSubscriber::GetAtomic(
    double defaultValue) const {
  return ::nt::GetAtomicDouble(m_subHandle, defaultValue);
}

inline std::vector<TimestampedDouble>
DoubleSubscriber::ReadQueue() {
  return ::nt::ReadQueueDouble(m_subHandle);
}

inline DoubleTopic DoubleSubscriber::GetTopic() const {
  return DoubleTopic{::nt::GetTopicFromHandle(m_subHandle)};
}

inline DoublePublisher::DoublePublisher(NT_Publisher handle)
    : Publisher{handle} {}

inline void DoublePublisher::Set(double value,
                                         int64_t time) {
  ::nt::SetDouble(m_pubHandle, value, time);
}

inline void DoublePublisher::SetDefault(double value) {
  ::nt::SetDefaultDouble(m_pubHandle, value);
}

inline DoubleTopic DoublePublisher::GetTopic() const {
  return DoubleTopic{::nt::GetTopicFromHandle(m_pubHandle)};
}

inline DoubleEntry::DoubleEntry(
    NT_Entry handle, double defaultValue)
    : DoubleSubscriber{handle, defaultValue},
      DoublePublisher{handle} {}

inline DoubleTopic DoubleEntry::GetTopic() const {
  return DoubleTopic{::nt::GetTopicFromHandle(m_subHandle)};
}

inline void DoubleEntry::Unpublish() {
  ::nt::Unpublish(m_pubHandle);
}

inline DoubleSubscriber DoubleTopic::Subscribe(
    double defaultValue,
    std::span<const PubSubOption> options) {
  return DoubleSubscriber{
      ::nt::Subscribe(m_handle, NT_DOUBLE, "double", options),
      defaultValue};
}
inline DoubleSubscriber DoubleTopic::SubscribeEx(
    std::string_view typeString, double defaultValue,
    std::span<const PubSubOption> options) {
  return DoubleSubscriber{
      ::nt::Subscribe(m_handle, NT_DOUBLE, typeString, options),
      defaultValue};
}

inline DoublePublisher DoubleTopic::Publish(
    std::span<const PubSubOption> options) {
  return DoublePublisher{
      ::nt::Publish(m_handle, NT_DOUBLE, "double", options)};
}

inline DoublePublisher DoubleTopic::PublishEx(
    std::string_view typeString,
    const wpi::json& properties, std::span<const PubSubOption> options) {
  return DoublePublisher{
      ::nt::PublishEx(m_handle, NT_DOUBLE, typeString, properties, options)};
}

inline DoubleEntry DoubleTopic::GetEntry(
    double defaultValue,
    std::span<const PubSubOption> options) {
  return DoubleEntry{
      ::nt::GetEntry(m_handle, NT_DOUBLE, "double", options),
      defaultValue};
}
inline DoubleEntry DoubleTopic::GetEntryEx(
    std::string_view typeString, double defaultValue,
    std::span<const PubSubOption> options) {
  return DoubleEntry{
      ::nt::GetEntry(m_handle, NT_DOUBLE, typeString, options),
      defaultValue};
}

}  // namespace nt