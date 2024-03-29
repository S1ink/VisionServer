/* Copyright 2020 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_CORE_TPU_TPU_API_DLSYM_INITIALIZER_H_
#define TENSORFLOW_CORE_TPU_TPU_API_DLSYM_INITIALIZER_H_

#include "tensorflow/core/platform/status.h"
#include "tensorflow/core/tpu/libtftpu.h"
#include "tensorflow/core/tpu/tpu_ops_c_api.h"
#include "tensorflow/stream_executor/tpu/tpu_executor_c_api.h"

// LINT.IfChange
namespace tensorflow {
namespace tpu {

Status InitializeTpuLibrary(void* library_handle);

}  // namespace tpu
}  // namespace tensorflow
// LINT.ThenChange(//tensorflow/core/tpu/tpu_api_dlsym_initializer_windows.cc)

#endif  // TENSORFLOW_CORE_TPU_TPU_API_DLSYM_INITIALIZER_H_
