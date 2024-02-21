#pragma once

#include <aug.h>

void aug_gfx_init(aug_vm* vm);
void aug_gfx_startup();
void aug_gfx_shutdown();
bool aug_gfx_update();

void aug_gfx_push_script(const char* script_filepath);
void aug_gfx_pop_script();

void aug_gfx_set_working_paths(const char* asset_path, const char* script_path);
