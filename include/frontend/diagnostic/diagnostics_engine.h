#pragma once
#include <map>
#include <string>
#include <utility>

namespace diagnostic {

// Each diagnostic level
enum class Level {
#define DiagnosticLevel(level) level,
#include "frontend/diagnostic/DiagnosticLevels.def"
#undef DiagnosticLevel
};

// Map each level to itsself
const static std::map<Level, Level> kDefaultLevelMapping = {
#define DiagnosticLevel(level) {Level::level, Level::level},
#include "frontend/diagnostic/DiagnosticLevels.def"
#undef DiagnosticLevel
};

class DiagnosticsEngine {
 public:
  static DiagnosticsEngine& get() {
    static DiagnosticsEngine diag_engine;
    return diag_engine;
  }

  // Allow changing warning levels (Wall)
  void setMapping(std::map<Level, Level> levelMap) {
    mapping_ = std::move(levelMap);
  }

  void emit(Level level, const std::string& msg) {
    Level mapped_level = mapping_[level];
    (void)mapped_level;
    (void)msg;
  }

 private:
  std::map<Level, Level> mapping_ = kDefaultLevelMapping;
};
}  // namespace diagnostic
