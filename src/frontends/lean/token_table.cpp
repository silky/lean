/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include <limits>
#include <utility>
#include "frontends/lean/token_table.h"

namespace lean {
static unsigned g_arrow_prec = 25;
static unsigned g_max_prec   = std::numeric_limits<unsigned>::max();
static unsigned g_plus_prec  = 65;
static unsigned g_cup_prec   = 60;
unsigned get_max_prec() { return g_max_prec; }
unsigned get_arrow_prec() { return g_arrow_prec; }
token_table add_command_token(token_table const & s, char const * token) {
    return insert(s, token, token_info(token));
}
token_table add_command_token(token_table const & s, char const * token, char const * val) {
    return insert(s, token, token_info(token, val));
}
token_table add_token(token_table const & s, char const * token, unsigned prec) {
    return insert(s, token, token_info(token, prec));
}
token_table add_token(token_table const & s, char const * token, char const * val, unsigned prec) {
    return insert(s, token, token_info(token, val, prec));
}
token_table const * find(token_table const & s, char c) {
    return s.find(c);
}
token_info const * value_of(token_table const & s) {
    return s.value();
}
optional<unsigned> get_precedence(token_table const & s, char const * token) {
    auto it = find(s, token);
    return it ? optional<unsigned>(it->precedence()) : optional<unsigned>();
}
void for_each(token_table const & s, std::function<void(char const *, token_info const &)> const & fn) {
    s.for_each([&](unsigned num, char const * keys, token_info const & info) {
            buffer<char> str;
            str.append(num, keys);
            str.push_back(0);
            fn(str.data(), info);
        });
}
void display(std::ostream & out, token_table const & s) {
    for_each(s, [&](char const * token, token_info const & info) {
            out << "`" << token << "`:" << info.precedence();
            if (info.is_command())
                out << " [command]";
            if (info.value() != info.token())
                out << " " << info.value();
            out << "\n";
        });
}

static char const * g_lambda_unicode = "\u03BB";
static char const * g_pi_unicode     = "\u03A0";
static char const * g_forall_unicode = "\u2200";
static char const * g_arrow_unicode  = "\u2192";
static char const * g_cup            = "\u2294";

token_table init_token_table() {
    token_table t;
    std::pair<char const *, unsigned> builtin[] =
        {{"fun", 0}, {"Pi", 0}, {"let", 0}, {"in", 0}, {"have", 0}, {"show", 0}, {"obtain", 0}, {"by", 0}, {"then", 0},
         {"from", 0}, {"(", g_max_prec}, {")", 0}, {"{", g_max_prec}, {"}", 0}, {"_", g_max_prec},
         {"[", g_max_prec}, {"]", 0}, {"⦃", g_max_prec}, {"⦄", 0}, {".{", 0}, {"Type", g_max_prec}, {"Type'", g_max_prec},
         {"|", 0}, {"!", 0}, {"with", 0}, {"...", 0}, {",", 0}, {".", 0}, {":", 0}, {"calc", 0}, {":=", 0}, {"--", 0}, {"#", 0},
         {"(*", 0}, {"(--", 0}, {"proof", 0}, {"qed", 0}, {"@", g_max_prec}, {"including", 0},
         {"+", g_plus_prec}, {g_cup, g_cup_prec}, {"->", g_arrow_prec}, {nullptr, 0}};

    char const * commands[] = {"theorem", "axiom", "variable", "definition", "coercion",
                               "variables", "[private]", "[inline]", "[fact]", "[instance]", "[class]", "[module]", "[coercion]",
                               "abbreviation", "opaque_hint", "evaluate", "check", "print", "end", "namespace", "section", "import",
                               "abbreviation", "inductive", "record", "structure", "module", "universe",
                               "precedence", "infixl", "infixr", "infix", "postfix", "prefix", "notation",
                               "exit", "set_option", "using", "calc_subst", "calc_refl", "calc_trans", "tactic_hint",
                               "add_proof_qed", "set_proof_qed", "#setline", "class", "instance", nullptr};

    std::pair<char const *, char const *> aliases[] =
        {{g_lambda_unicode, "fun"}, {"forall", "Pi"}, {g_forall_unicode, "Pi"}, {g_pi_unicode, "Pi"},
         {nullptr, nullptr}};

    std::pair<char const *, char const *> cmd_aliases[] =
        {{"parameter", "variable"}, {"parameters", "variables"}, {"lemma", "theorem"},
         {"hypothesis", "axiom"}, {"conjecture", "axiom"}, {"corollary", "theorem"},
         {nullptr, nullptr}};

    auto it = builtin;
    while (it->first) {
        t = add_token(t, it->first, it->second);
        it++;
    }

    auto it2 = commands;
    while (*it2) {
        t = add_command_token(t, *it2);
        ++it2;
    }

    auto it3 = aliases;
    while (it3->first) {
        t = add_token(t, it3->first, it3->second);
        it3++;
    }
    t = add_token(t, g_arrow_unicode, "->", get_arrow_prec());

    auto it4 = cmd_aliases;
    while (it4->first) {
        t = add_command_token(t, it4->first, it4->second);
        ++it4;
    }
    return t;
}

token_table mk_default_token_table() {
    static optional<token_table> r;
    if (!r)
        r = init_token_table();
    return *r;
}

static token_table g_init(mk_default_token_table());

token_table mk_token_table() { return token_table(); }

DECL_UDATA(token_table)
static int mk_token_table(lua_State * L) { return push_token_table(L, mk_token_table()); }
static int mk_default_token_table(lua_State * L) { return push_token_table(L, mk_default_token_table()); }
static int add_command_token(lua_State * L) {
    int nargs = lua_gettop(L);
    if (nargs == 2)
        return push_token_table(L, add_command_token(to_token_table(L, 1), lua_tostring(L, 2)));
    else
        return push_token_table(L, add_command_token(to_token_table(L, 1), lua_tostring(L, 2), lua_tostring(L, 3)));
}
static int add_token(lua_State * L) {
    int nargs = lua_gettop(L);
    if (nargs == 3)
        return push_token_table(L, add_token(to_token_table(L, 1), lua_tostring(L, 2), lua_tonumber(L, 3)));
    else
        return push_token_table(L, add_token(to_token_table(L, 1), lua_tostring(L, 2), lua_tostring(L, 3), lua_tonumber(L, 4)));
}
static int merge(lua_State * L) {
    return push_token_table(L, merge(to_token_table(L, 1), to_token_table(L, 2)));
}
static int find(lua_State * L) {
    char k;
    if (lua_isnumber(L, 2)) {
        k = lua_tonumber(L, 2);
    } else {
        char const * str = lua_tostring(L, 2);
        if (strlen(str) != 1)
            throw exception("arg #2 must be a string of length 1");
        k = str[0];
    }
    auto it = to_token_table(L, 1).find(k);
    if (it)
        return push_token_table(L, *it);
    else
        return push_nil(L);
}
static int value_of(lua_State * L) {
    auto it = value_of(to_token_table(L, 1));
    if (it) {
        push_boolean(L, it->is_command());
        push_name(L, it->value());
        push_integer(L, it->precedence());
        return 3;
    } else {
        push_nil(L);
        return 1;
    }
}
static int for_each(lua_State * L) {
    token_table const & t = to_token_table(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION); // user-fun
    for_each(t, [&](char const * k, token_info const & info) {
            lua_pushvalue(L, 2);
            lua_pushstring(L, k);
            lua_pushboolean(L, info.is_command());
            push_name(L, info.value());
            lua_pushinteger(L, info.precedence());
            pcall(L, 4, 0, 0);
        });
    return 0;
}

static const struct luaL_Reg token_table_m[] = {
    {"__gc",              token_table_gc},
    {"add_command_token", safe_function<add_command_token>},
    {"add_token",         safe_function<add_token>},
    {"merge",             safe_function<merge>},
    {"find",              safe_function<find>},
    {"value_of",          safe_function<value_of>},
    {"for_each",          safe_function<for_each>},
    {0, 0}
};

void open_token_table(lua_State * L) {
    luaL_newmetatable(L, token_table_mt);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    setfuncs(L, token_table_m, 0);

    SET_GLOBAL_FUN(token_table_pred,       "is_token_table");
    SET_GLOBAL_FUN(mk_default_token_table, "default_token_table");
    SET_GLOBAL_FUN(mk_token_table,         "token_table");
}
}
