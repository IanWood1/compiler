#pragma once

#include "frontend/ast/ast.h"
namespace frontend {
Program parse_file(const char* file_name);
}
