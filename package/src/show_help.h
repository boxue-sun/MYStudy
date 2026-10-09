#pragma once

#include <string>

namespace airos {
namespace pack {

void show_help();
void show_help_create();
void show_help_make();
void show_help_release();
void show_help_install();
void show_help_run();
void show_help_remove();
bool have_help();
void show_args(const std::string& type);

}  // namespace pack
}  // namespace airos
