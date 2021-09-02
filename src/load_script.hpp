/*
 * Copyright (c) 2021, Dylam De La Torre <dyxel04@gmail.com>
 *
 * SPDX-License-Identifier: AGPL-3.0-or-later
 */
#include <string>
#include <string_view>

auto load_script(void*, std::string_view name) noexcept -> std::string;
