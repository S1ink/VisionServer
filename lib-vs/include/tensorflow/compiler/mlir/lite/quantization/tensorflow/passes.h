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

#ifndef TENSORFLOW_COMPILER_MLIR_LITE_QUANTIZATION_TENSORFLOW_PASSES_H_
#define TENSORFLOW_COMPILER_MLIR_LITE_QUANTIZATION_TENSORFLOW_PASSES_H_

#include <memory>

#include "mlir/Dialect/Func/IR/FuncOps.h"  // from @llvm-project
#include "mlir/IR/BuiltinOps.h"  // from @llvm-project
#include "mlir/Pass/Pass.h"  // from @llvm-project

namespace mlir {
namespace TF {

// Legalize the tf ops to the quant ops, so the quantization passes can work.
std::unique_ptr<OperationPass<FuncOp>> CreateLegalizeTFToQuantPass();

// Fallbacks ops that are not supported by TF Quantization to TFLite Flex ops.
std::unique_ptr<OperationPass<FuncOp>> CreateFallbackToFlexOpsPass(
    const std::string &mode = "DEFAULT");

}  // namespace TF
}  // namespace mlir
#endif  // TENSORFLOW_COMPILER_MLIR_LITE_QUANTIZATION_TENSORFLOW_PASSES_H_
