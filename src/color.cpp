// Copyright (c) 2013 by Igor Kalnitsky.
// All rights reserved.
//
// Redistribution and use in source and binary forms of the software as well
// as documentation, with or without modification, are permitted provided
// that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above
//   copyright notice, this list of conditions and the following
//   disclaimer in the documentation and/or other materials provided
//   with the distribution.
//
// * The names of the contributors may not be used to endorse or
//   promote products derived from this software without specific
//   prior written permission.
//
// THIS SOFTWARE AND DOCUMENTATION IS PROVIDED BY THE COPYRIGHT HOLDERS AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
// OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE AND DOCUMENTATION, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.

#include <ice/color.h>

#if defined(_WIN32) || defined(_WIN64)
# define OS_WINDOWS
#elif defined(__APPLE__)
# define OS_MACOS
#elif defined(__unix__) || defined(__unix)
# define OS_UNIX
#else
# error unsupported platform
#endif

#if defined(OS_MACOS) || defined(OS_UNIX)
# include <unistd.h>
#elif defined(OS_WINDOWS)
# include <io.h>
# include <windows.h>
#endif

#include <iostream>
#include <cstdio>

#if !defined(OS_WINDOWS)
#define FOREGROUND_RED -1
#define FOREGROUND_GREEN -1
#define FOREGROUND_BLUE -1
#define BACKGROUND_RED -1
#define BACKGROUND_GREEN -1
#define BACKGROUND_BLUE -1
#endif

namespace ice {
namespace color {
namespace {

enum class color_type {
  none,
  unix,
#if defined(OS_WINDOWS)
  windows,
#endif
};

#if defined(OS_MACOS) || defined(OS_UNIX)

FILE* get_standard_stream(const std::ostream& stream)
{
  if (&stream == &std::cout) {
    return stdout;
  } else if ((&stream == &std::cerr) || (&stream == &std::clog)) {
    return stderr;
  }
  return 0;
}

#elif defined(OS_WINDOWS)

HANDLE get_standard_handle(const std::ostream& stream)
{
  if (&stream == &std::cout) {
    return GetStdHandle(STD_OUTPUT_HANDLE);
  } else if ((&stream == &std::cerr) || (&stream == &std::clog)) {
    return GetStdHandle(STD_ERROR_HANDLE);
  }
  return INVALID_HANDLE_VALUE;
}

#endif

color_type get_color_type(const std::ostream& stream)
{
#if defined(OS_MACOS) || defined(OS_UNIX)
  if (isatty(fileno(get_standard_stream(stream)))) {
    return color_type::unix;
  }
#elif defined(OS_WINDOWS)
  auto type = GetFileType(get_standard_handle(stream));
  switch (type) {
  case 1: return color_type::none;     // Command Prompt (Pipe) || MinTTY (Pipe)
  case 2: return color_type::windows;  // Command Prompt
  case 3: return color_type::unix;     // MinTTY
  }
#endif
  return color_type::none;
}

#if defined(OS_WINDOWS)

// Change Windows Terminal colors attribute. If a parameter is `-1` then the attribute won't changed.
void change_attributes(std::ostream& stream, int foreground, int background)
{
  static WORD default_attributes = 0;
  static bool dark = false;

  // Get terminal handle.
  auto handle = get_standard_handle(stream);

  // Save default terminal attributes.
  if (!default_attributes) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(handle, &info)) {
      return;
    }
    default_attributes = info.wAttributes;
  }

  // Restore default settings.
  if (foreground == -1 && background == -1) {
    SetConsoleTextAttribute(handle, default_attributes);
    dark = false;
    return;
  }

  // Get current settings.
  CONSOLE_SCREEN_BUFFER_INFO info;
  if (!GetConsoleScreenBufferInfo(handle, &info)) {
    return;
  }

  if (foreground == -2) {
    foreground = -1;
    dark = true;
  }

  if (foreground != -1) {
    if (!dark) {
      foreground |= FOREGROUND_INTENSITY;
    }
    info.wAttributes &= ~(info.wAttributes & 0x0F);
    info.wAttributes |= static_cast<WORD>(foreground);
  }

  if (background != -1) {
    info.wAttributes &= ~(info.wAttributes & 0xF0);
    info.wAttributes |= static_cast<WORD>(background | BACKGROUND_INTENSITY);
  }

  SetConsoleTextAttribute(handle, info.wAttributes);
}

#endif // OS_WINDOWS

inline std::ostream& set_color(std::ostream& stream, const char* es, int foreground = -1, int background = -1)
{
  auto type = get_color_type(stream);
  switch (type) {
  case color_type::none:
    break;
  case color_type::unix:
    if (es) {
      stream << es;
    }
    break;
#if defined(OS_WINDOWS)
  case color_type::windows:
    change_attributes(stream, foreground, background);
    break;
#endif
  }
  return stream;
}

}  // namespace

std::ostream& bold(std::ostream& stream)
{
  return set_color(stream, "\033[1m");
}

std::ostream& dark(std::ostream& stream)
{
  return set_color(stream, "\033[2m", -2);
}

std::ostream& underline(std::ostream& stream)
{
  return set_color(stream, "\033[4m");
}

std::ostream& blink(std::ostream& stream)
{
  return set_color(stream, "\033[5m");
}

std::ostream& reverse(std::ostream& stream)
{
  return set_color(stream, "\033[7m");
}

std::ostream& concealed(std::ostream& stream)
{
  return set_color(stream, "\033[8m");
}

std::ostream& grey(std::ostream& stream)
{
  return set_color(stream, "\033[1;30m", 0);
}

std::ostream& red(std::ostream& stream)
{
  return set_color(stream, "\033[31m", FOREGROUND_RED);
}

std::ostream& green(std::ostream& stream)
{
  return set_color(stream, "\033[32m", FOREGROUND_GREEN);
}

std::ostream& yellow(std::ostream& stream)
{
  return set_color(stream, "\033[33m", FOREGROUND_GREEN | FOREGROUND_RED);
}

std::ostream& blue(std::ostream& stream)
{
  return set_color(stream, "\033[34m", FOREGROUND_BLUE);
}

std::ostream& magenta(std::ostream& stream)
{
  return set_color(stream, "\033[35m", FOREGROUND_BLUE | FOREGROUND_RED);
}

std::ostream& cyan(std::ostream& stream)
{
  return set_color(stream, "\033[36m", FOREGROUND_BLUE | FOREGROUND_GREEN);
}

std::ostream& white(std::ostream& stream)
{
  return set_color(stream, "\033[37m", FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

std::ostream& on_grey(std::ostream& stream)
{
  return set_color(stream, "\033[40m", -1, 0);
}

std::ostream& on_red(std::ostream& stream)
{
  return set_color(stream, "\033[41m", -1, BACKGROUND_RED);
}

std::ostream& on_green(std::ostream& stream)
{
  return set_color(stream, "\033[42m", -1, BACKGROUND_GREEN);
}

std::ostream& on_yellow(std::ostream& stream)
{
  return set_color(stream, "\033[43m", -1, BACKGROUND_GREEN | BACKGROUND_RED);
}

std::ostream& on_blue(std::ostream& stream)
{
  return set_color(stream, "\033[44m", -1, BACKGROUND_BLUE);
}

std::ostream& on_magenta(std::ostream& stream)
{
  return set_color(stream, "\033[45m", -1, BACKGROUND_BLUE | BACKGROUND_RED);
}

std::ostream& on_cyan(std::ostream& stream)
{
  return set_color(stream, "\033[46m", -1, BACKGROUND_GREEN | BACKGROUND_BLUE);
}

std::ostream& on_white(std::ostream& stream)
{
  return set_color(stream, "\033[47m", -1, BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_RED);
}

std::ostream& reset(std::ostream& stream)
{
  return set_color(stream, "\033[00m", -1, -1);
}

}  // namespace color
}  // namespace ice
