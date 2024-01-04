/*
The design of the DiagnosticIds and related classes is inspired by Clang/LLVM, with
 many features removed due to the smaller scale of this project.
The original source code can be found at https://github.com/llvm/llvm-project/tree/main/clang/include/clang/Basic
*/

#pragma once

namespace diagnostic {
enum class DiagnosticId {
  // Logging
  LogMessage,

  // Source Errors
  InvalidSyntax,

};

class DiagnosticIds {};
}  // namespace diagnostic