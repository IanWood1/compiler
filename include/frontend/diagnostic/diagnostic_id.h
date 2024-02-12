#pragma once

namespace diagnostic {
enum class DiagnosticId {
  // Logging
  DebugMsg,

  // Warnings
  SyntaxWarning,

  // Errors
  SyntaxError,
  TypeError,

};

class DiagnosticIds {};
}  // namespace diagnostic