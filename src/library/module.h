/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#pragma once
#include <string>
#include <iostream>
#include "util/serializer.h"
#include "kernel/inductive/inductive.h"
#include "library/shared_environment.h"
#include "library/io_state.h"

namespace lean {
/**
   \brief Return an environment based on \c env, where all modules in \c modules are imported.
   Modules included directly or indirectly by them are also imported.
   The environment \c env is usually an empty environment.

   If \c keep_proofs is false, then the proof of the imported theorems is discarded after being
   checked. The idea is to save memory.
*/
environment import_modules(environment const & env, unsigned num_modules, name const * modules,
                           unsigned num_threads, bool keep_proofs, io_state const & ios);
environment import_module(environment const & env, name const & module,
                          unsigned num_threads, bool keep_proofs, io_state const & ios);

/**
   \brief Store/Export module using \c env to the output stream \c out.
*/
void export_module(std::ostream & out, environment const & env);

/** \brief An asynchronous update. It goes into a task queue, and can be executed by a different execution thread. */
typedef std::function<void(shared_environment & env)> asynch_update_fn;

/** \brief Delayed update. It is performed after all imported modules have been loaded.
    The delayes updates are executed based on the import order.
    Example: if module A was imported before B, then delayed updates from A
    are executed before the ones from B.
*/
typedef std::function<environment(environment const & env, io_state const & ios)> delayed_update_fn;

/**
   \brief A reader for importing data from a stream using deserializer \c d.
   There are three ways to update the environment being constructed.
     1- Direct update it using \c senv.
     2- Asynchronous update using add_asynch_update.
     3- Delayed update using add_delayed_update.
*/
typedef void (*module_object_reader)(deserializer & d, module_idx midx, shared_environment & senv,
                                     std::function<void(asynch_update_fn const &)> & add_asynch_update,
                                     std::function<void(delayed_update_fn const &)> & add_delayed_update);

/**
   \brief Register a module object reader. The key \c k is used to identify the class of objects
   that can be read by the given reader.
*/
void register_module_object_reader(std::string const & k, module_object_reader r);

/** \brief Auxiliary class for registering module readers when the lean executable is loaded. */
struct register_module_object_reader_fn {
    register_module_object_reader_fn(std::string const & k, module_object_reader r) {
        register_module_object_reader(k, r);
    }
};

namespace module {
/**
    \brief Add a function that should be invoked when the environment is exported.
    The key \c k identifies which module_object_reader should be used to deserialize the object
    when the module is imported.

    \see module_object_reader
*/
environment add(environment const & env, std::string const & k, std::function<void(serializer &)> const & writer);

/** \brief Add the global universe declaration to the environment, and mark it to be exported. */
environment add_universe(environment const & env, name const & l);

/** \brief Add the given declaration to the environment, and mark it to be exported. */
environment add(environment const & env, certified_declaration const & d);
/**
    \brief Add the given declaration to the environment, and mark it to be exported.
    This method throws an exception if the trust_level <= LEAN_BELIEVER_TRUST_LEVEL
*/
environment add(environment const & env, declaration const & d);

/** \brief Add the given inductive declaration to the environment, and mark it to be exported. */
environment add_inductive(environment                  env,
                          level_param_names const &    level_params,
                          unsigned                     num_params,
                          list<inductive::inductive_decl> const & decls);

/**
   \brief Declare a single inductive datatype. This is just a helper function implemented on top of
    the previous (more general) add_inductive.
*/
environment add_inductive(environment const &        env,
                          name const &               ind_name,         // name of new inductive datatype
                          level_param_names const &  level_params,     // level parameters
                          unsigned                   num_params,       // number of params
                          expr const &               type,             // type of the form: params -> indices -> Type
                          list<inductive::intro_rule> const & intro_rules);     // introduction rules

}
}
