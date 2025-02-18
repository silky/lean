/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include <fstream>
#include "kernel/environment.h"

namespace lean {
/** \brief Display in \c out all files the .lean file \c fname depends on */
void display_deps(environment const & env, std::ostream & out, char const * fname);
}
