// Copyright (c) FIRST and other WPILib contributors.
// Open Source Software; you can modify and/or share it under the terms of
// the WPILib BSD license file in the root directory of this project.

#pragma once

#include <stdint.h>

#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ntcore_c.h"

namespace wpi {
template <typename T>
class SmallVectorImpl;
}  // namespace wpi

namespace nt {

/**
 * Timestamped Boolean.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedBoolean {
  TimestampedBoolean() = default;
  TimestampedBoolean(int64_t time, int64_t serverTime, bool value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  bool value = {};
};

/**
 * @defgroup ntcore_Boolean_func Boolean Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetBoolean(NT_Handle pubentry, bool value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultBoolean(NT_Handle pubentry, bool defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
bool GetBoolean(NT_Handle subentry, bool defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedBoolean GetAtomicBoolean(NT_Handle subentry, bool defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedBoolean> ReadQueueBoolean(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<int> ReadQueueValuesBoolean(NT_Handle subentry);

/** @} */

/**
 * Timestamped Integer.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedInteger {
  TimestampedInteger() = default;
  TimestampedInteger(int64_t time, int64_t serverTime, int64_t value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  int64_t value = {};
};

/**
 * @defgroup ntcore_Integer_func Integer Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetInteger(NT_Handle pubentry, int64_t value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultInteger(NT_Handle pubentry, int64_t defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
int64_t GetInteger(NT_Handle subentry, int64_t defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedInteger GetAtomicInteger(NT_Handle subentry, int64_t defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedInteger> ReadQueueInteger(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<int64_t> ReadQueueValuesInteger(NT_Handle subentry);

/** @} */

/**
 * Timestamped Float.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedFloat {
  TimestampedFloat() = default;
  TimestampedFloat(int64_t time, int64_t serverTime, float value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  float value = {};
};

/**
 * @defgroup ntcore_Float_func Float Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetFloat(NT_Handle pubentry, float value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultFloat(NT_Handle pubentry, float defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
float GetFloat(NT_Handle subentry, float defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedFloat GetAtomicFloat(NT_Handle subentry, float defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedFloat> ReadQueueFloat(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<float> ReadQueueValuesFloat(NT_Handle subentry);

/** @} */

/**
 * Timestamped Double.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedDouble {
  TimestampedDouble() = default;
  TimestampedDouble(int64_t time, int64_t serverTime, double value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  double value = {};
};

/**
 * @defgroup ntcore_Double_func Double Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetDouble(NT_Handle pubentry, double value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultDouble(NT_Handle pubentry, double defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
double GetDouble(NT_Handle subentry, double defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedDouble GetAtomicDouble(NT_Handle subentry, double defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedDouble> ReadQueueDouble(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<double> ReadQueueValuesDouble(NT_Handle subentry);

/** @} */

/**
 * Timestamped String.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedString {
  TimestampedString() = default;
  TimestampedString(int64_t time, int64_t serverTime, std::string value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::string value = {};
};

/**
 * Timestamped String view (for SmallVector-taking functions).
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedStringView {
  TimestampedStringView() = default;
  TimestampedStringView(int64_t time, int64_t serverTime, std::string_view value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::string_view value = {};
};

/**
 * @defgroup ntcore_String_func String Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetString(NT_Handle pubentry, std::string_view value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultString(NT_Handle pubentry, std::string_view defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
std::string GetString(NT_Handle subentry, std::string_view defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedString GetAtomicString(NT_Handle subentry, std::string_view defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedString> ReadQueueString(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<std::string> ReadQueueValuesString(NT_Handle subentry);

std::string_view GetString(NT_Handle subentry, wpi::SmallVectorImpl<char>& buf, std::string_view defaultValue);

TimestampedStringView GetAtomicString(
      NT_Handle subentry,
      wpi::SmallVectorImpl<char>& buf,
      std::string_view defaultValue);

/** @} */

/**
 * Timestamped Raw.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedRaw {
  TimestampedRaw() = default;
  TimestampedRaw(int64_t time, int64_t serverTime, std::vector<uint8_t> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::vector<uint8_t> value = {};
};

/**
 * Timestamped Raw view (for SmallVector-taking functions).
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedRawView {
  TimestampedRawView() = default;
  TimestampedRawView(int64_t time, int64_t serverTime, std::span<uint8_t> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::span<uint8_t> value = {};
};

/**
 * @defgroup ntcore_Raw_func Raw Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetRaw(NT_Handle pubentry, std::span<const uint8_t> value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultRaw(NT_Handle pubentry, std::span<const uint8_t> defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
std::vector<uint8_t> GetRaw(NT_Handle subentry, std::span<const uint8_t> defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedRaw GetAtomicRaw(NT_Handle subentry, std::span<const uint8_t> defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedRaw> ReadQueueRaw(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<std::vector<uint8_t>> ReadQueueValuesRaw(NT_Handle subentry);

std::span<uint8_t> GetRaw(NT_Handle subentry, wpi::SmallVectorImpl<uint8_t>& buf, std::span<const uint8_t> defaultValue);

TimestampedRawView GetAtomicRaw(
      NT_Handle subentry,
      wpi::SmallVectorImpl<uint8_t>& buf,
      std::span<const uint8_t> defaultValue);

/** @} */

/**
 * Timestamped BooleanArray.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedBooleanArray {
  TimestampedBooleanArray() = default;
  TimestampedBooleanArray(int64_t time, int64_t serverTime, std::vector<int> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::vector<int> value = {};
};

/**
 * Timestamped BooleanArray view (for SmallVector-taking functions).
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedBooleanArrayView {
  TimestampedBooleanArrayView() = default;
  TimestampedBooleanArrayView(int64_t time, int64_t serverTime, std::span<int> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::span<int> value = {};
};

/**
 * @defgroup ntcore_BooleanArray_func BooleanArray Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetBooleanArray(NT_Handle pubentry, std::span<const int> value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultBooleanArray(NT_Handle pubentry, std::span<const int> defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
std::vector<int> GetBooleanArray(NT_Handle subentry, std::span<const int> defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedBooleanArray GetAtomicBooleanArray(NT_Handle subentry, std::span<const int> defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedBooleanArray> ReadQueueBooleanArray(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<std::vector<int>> ReadQueueValuesBooleanArray(NT_Handle subentry);

std::span<int> GetBooleanArray(NT_Handle subentry, wpi::SmallVectorImpl<int>& buf, std::span<const int> defaultValue);

TimestampedBooleanArrayView GetAtomicBooleanArray(
      NT_Handle subentry,
      wpi::SmallVectorImpl<int>& buf,
      std::span<const int> defaultValue);

/** @} */

/**
 * Timestamped IntegerArray.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedIntegerArray {
  TimestampedIntegerArray() = default;
  TimestampedIntegerArray(int64_t time, int64_t serverTime, std::vector<int64_t> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::vector<int64_t> value = {};
};

/**
 * Timestamped IntegerArray view (for SmallVector-taking functions).
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedIntegerArrayView {
  TimestampedIntegerArrayView() = default;
  TimestampedIntegerArrayView(int64_t time, int64_t serverTime, std::span<int64_t> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::span<int64_t> value = {};
};

/**
 * @defgroup ntcore_IntegerArray_func IntegerArray Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetIntegerArray(NT_Handle pubentry, std::span<const int64_t> value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultIntegerArray(NT_Handle pubentry, std::span<const int64_t> defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
std::vector<int64_t> GetIntegerArray(NT_Handle subentry, std::span<const int64_t> defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedIntegerArray GetAtomicIntegerArray(NT_Handle subentry, std::span<const int64_t> defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedIntegerArray> ReadQueueIntegerArray(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<std::vector<int64_t>> ReadQueueValuesIntegerArray(NT_Handle subentry);

std::span<int64_t> GetIntegerArray(NT_Handle subentry, wpi::SmallVectorImpl<int64_t>& buf, std::span<const int64_t> defaultValue);

TimestampedIntegerArrayView GetAtomicIntegerArray(
      NT_Handle subentry,
      wpi::SmallVectorImpl<int64_t>& buf,
      std::span<const int64_t> defaultValue);

/** @} */

/**
 * Timestamped FloatArray.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedFloatArray {
  TimestampedFloatArray() = default;
  TimestampedFloatArray(int64_t time, int64_t serverTime, std::vector<float> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::vector<float> value = {};
};

/**
 * Timestamped FloatArray view (for SmallVector-taking functions).
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedFloatArrayView {
  TimestampedFloatArrayView() = default;
  TimestampedFloatArrayView(int64_t time, int64_t serverTime, std::span<float> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::span<float> value = {};
};

/**
 * @defgroup ntcore_FloatArray_func FloatArray Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetFloatArray(NT_Handle pubentry, std::span<const float> value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultFloatArray(NT_Handle pubentry, std::span<const float> defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
std::vector<float> GetFloatArray(NT_Handle subentry, std::span<const float> defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedFloatArray GetAtomicFloatArray(NT_Handle subentry, std::span<const float> defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedFloatArray> ReadQueueFloatArray(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<std::vector<float>> ReadQueueValuesFloatArray(NT_Handle subentry);

std::span<float> GetFloatArray(NT_Handle subentry, wpi::SmallVectorImpl<float>& buf, std::span<const float> defaultValue);

TimestampedFloatArrayView GetAtomicFloatArray(
      NT_Handle subentry,
      wpi::SmallVectorImpl<float>& buf,
      std::span<const float> defaultValue);

/** @} */

/**
 * Timestamped DoubleArray.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedDoubleArray {
  TimestampedDoubleArray() = default;
  TimestampedDoubleArray(int64_t time, int64_t serverTime, std::vector<double> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::vector<double> value = {};
};

/**
 * Timestamped DoubleArray view (for SmallVector-taking functions).
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedDoubleArrayView {
  TimestampedDoubleArrayView() = default;
  TimestampedDoubleArrayView(int64_t time, int64_t serverTime, std::span<double> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::span<double> value = {};
};

/**
 * @defgroup ntcore_DoubleArray_func DoubleArray Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetDoubleArray(NT_Handle pubentry, std::span<const double> value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultDoubleArray(NT_Handle pubentry, std::span<const double> defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
std::vector<double> GetDoubleArray(NT_Handle subentry, std::span<const double> defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedDoubleArray GetAtomicDoubleArray(NT_Handle subentry, std::span<const double> defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedDoubleArray> ReadQueueDoubleArray(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<std::vector<double>> ReadQueueValuesDoubleArray(NT_Handle subentry);

std::span<double> GetDoubleArray(NT_Handle subentry, wpi::SmallVectorImpl<double>& buf, std::span<const double> defaultValue);

TimestampedDoubleArrayView GetAtomicDoubleArray(
      NT_Handle subentry,
      wpi::SmallVectorImpl<double>& buf,
      std::span<const double> defaultValue);

/** @} */

/**
 * Timestamped StringArray.
 * @ingroup ntcore_cpp_handle_api
 */
struct TimestampedStringArray {
  TimestampedStringArray() = default;
  TimestampedStringArray(int64_t time, int64_t serverTime, std::vector<std::string> value)
    : time{time}, serverTime{serverTime}, value{std::move(value)} {}

  /**
   * Time in local time base.
   */
  int64_t time = 0;

  /**
   * Time in server time base.  May be 0 or 1 for locally set values.
   */
  int64_t serverTime = 0;

  /**
   * Value.
   */
  std::vector<std::string> value = {};
};

/**
 * @defgroup ntcore_StringArray_func StringArray Functions
 * @ingroup ntcore_cpp_handle_api
 * @{
 */

/**
 * Publish a new value.
 *
 * @param pubentry publisher or entry handle
 * @param value value to publish
 * @param time timestamp; 0 indicates current NT time should be used
 */
bool SetStringArray(NT_Handle pubentry, std::span<const std::string> value, int64_t time = 0);

/**
 * Publish a default value.
 * On reconnect, a default value will never be used in preference to a
 * published value.
 *
 * @param pubentry publisher or entry handle
 * @param defaultValue default value
 */
bool SetDefaultStringArray(NT_Handle pubentry, std::span<const std::string> defaultValue);

/**
 * Get the last published value.
 * If no value has been published, returns the passed defaultValue.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return value
 */
std::vector<std::string> GetStringArray(NT_Handle subentry, std::span<const std::string> defaultValue);

/**
 * Get the last published value along with its timestamp.
 * If no value has been published, returns the passed defaultValue and a
 * timestamp of 0.
 *
 * @param subentry subscriber or entry handle
 * @param defaultValue default value to return if no value has been published
 * @return timestamped value
 */
TimestampedStringArray GetAtomicStringArray(NT_Handle subentry, std::span<const std::string> defaultValue);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 * Also provides a timestamp for each value.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of timestamped values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<TimestampedStringArray> ReadQueueStringArray(NT_Handle subentry);

/**
 * Get an array of all value changes since the last call to ReadQueue.
 *
 * @note The "poll storage" subscribe option can be used to set the queue
 *     depth.
 *
 * @param subentry subscriber or entry handle
 * @return Array of values; empty array if no new changes have
 *     been published since the previous call.
 */
std::vector<std::vector<std::string>> ReadQueueValuesStringArray(NT_Handle subentry);

/** @} */

}  // namespace nt