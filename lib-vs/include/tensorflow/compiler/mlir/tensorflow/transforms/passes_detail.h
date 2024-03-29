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

#ifndef TENSORFLOW_COMPILER_MLIR_TENSORFLOW_TRANSFORMS_TF_PASS_DETAIL_H_
#define TENSORFLOW_COMPILER_MLIR_TENSORFLOW_TRANSFORMS_TF_PASS_DETAIL_H_

#include "mlir/IR/Dialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"  // from @llvm-project
#include "tensorflow/compiler/mlir/tensorflow/ir/tf_executor.h"

namespace mlir {
namespace scf {
class SCFDialect;
}
namespace tensor {
class TensorDialect;
}
namespace tf_device {
class TensorFlowDeviceDialect;
}
namespace tf_executor {
class TensorFlowExecutorDialect;
}
namespace TF {
class TensorFlowDialect;
// Direction in which to move transposes in MoveTransposePass.
enum MoveTransposeDirection { kBegin, kEnd };

#define GEN_PASS_CLASSES
#include "tensorflow/compiler/mlir/tensorflow/transforms/tf_passes.h.inc"

}  // namespace TF
}  // namespace mlir

#endif  // TENSORFLOW_COMPILER_MLIR_TENSORFLOW_TRANSFORMS_TF_PASS_DETAIL_H_
