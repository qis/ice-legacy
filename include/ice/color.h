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

#pragma once
#include <iostream>

namespace ice {
namespace color {

std::ostream& bold(std::ostream& stream);
std::ostream& dark(std::ostream& stream);
std::ostream& underline(std::ostream& stream);
std::ostream& blink(std::ostream& stream);
std::ostream& reverse(std::ostream& stream);
std::ostream& concealed(std::ostream& stream);

std::ostream& grey(std::ostream& stream);
std::ostream& red(std::ostream& stream);
std::ostream& green(std::ostream& stream);
std::ostream& yellow(std::ostream& stream);
std::ostream& blue(std::ostream& stream);
std::ostream& magenta(std::ostream& stream);
std::ostream& cyan(std::ostream& stream);
std::ostream& white(std::ostream& stream);

std::ostream& on_grey(std::ostream& stream);
std::ostream& on_red(std::ostream& stream);
std::ostream& on_green(std::ostream& stream);
std::ostream& on_yellow(std::ostream& stream);
std::ostream& on_blue(std::ostream& stream);
std::ostream& on_magenta(std::ostream& stream);
std::ostream& on_cyan(std::ostream& stream);
std::ostream& on_white(std::ostream& stream);

std::ostream& reset(std::ostream& stream);

}  // namespace color
}  // namespace ice
