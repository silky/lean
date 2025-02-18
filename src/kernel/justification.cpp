/*
Copyright (c) 2013 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include <string>
#include <vector>
#include "util/buffer.h"
#include "util/int64.h"
#include "util/memory_pool.h"
#include "kernel/justification.h"
#include "kernel/metavar.h"

#ifndef LEAN_DEFAULT_DEPENDS_ON_CACHE_CAPACITY
#define LEAN_DEFAULT_DEPENDS_ON_CACHE_CAPACITY 1024*8
#endif

namespace lean {
format to_pos(optional<expr> const & e, pos_info_provider const * p) {
    if (!p || !e)
        return format();
    format f = p->pp(*e);
    if (!f)
        return format();
    return f + space();
}

typedef uint64 approx_set;
static approx_set mk_empty_set() { return 0; }
static approx_set mk_union(approx_set s1, approx_set s2) { return s1 | s2; }
static approx_set mk_intersection(approx_set s1, approx_set s2) { return s1 & s2; }
static approx_set mk_singleton(unsigned i) { return static_cast<uint64>(1) << (i % 64); }
static approx_set may_contain(approx_set s, unsigned i) { return mk_intersection(s, mk_singleton(i)) != 0ull; }

enum class justification_kind { Asserted, Composite, ExtComposite, Assumption, ExtAssumption };

approx_set get_approx_assumption_set(justification const & j);

MK_THREAD_LOCAL_GET(unsigned, get_hash_alloc_jst_counter, 0)

struct justification_cell {
    MK_LEAN_RC();
    justification_kind m_kind;
    unsigned           m_hash_alloc;
    void dealloc();
    justification_cell(justification_kind k):m_rc(0), m_kind(k) {
        m_hash_alloc = get_hash_alloc_jst_counter();
        get_hash_alloc_jst_counter()++;
    }
    bool is_asserted() const { return m_kind == justification_kind::Asserted; }
    bool is_assumption() const { return m_kind == justification_kind::Assumption || m_kind == justification_kind::ExtAssumption; }
    bool is_composite() const { return m_kind == justification_kind::Composite || m_kind == justification_kind::ExtComposite; }
    bool is_ext_assumption() const { return m_kind == justification_kind::ExtAssumption; }
    bool is_ext_composite() const { return m_kind == justification_kind::ExtComposite; }
};

struct asserted_cell : public justification_cell {
    pp_jst_fn      m_fn;
    optional<expr> m_expr;
    asserted_cell(pp_jst_fn const & fn, optional<expr> const & e):
        justification_cell(justification_kind::Asserted),
        m_fn(fn), m_expr(e) {}
};

struct composite_cell : public justification_cell {
    approx_set    m_assumption_set; // approximated set of assumptions contained in child1 and child2
    justification m_child[2];
    composite_cell(justification_kind k, justification const & j1, justification const & j2):
        justification_cell(k) {
        m_child[0] = j1;
        m_child[1] = j2;
        m_assumption_set = mk_union(get_approx_assumption_set(j1), get_approx_assumption_set(j2));
    }
    composite_cell(justification const & j1, justification const & j2):
        composite_cell(justification_kind::Composite, j1, j2) {}
};

struct ext_composite_cell : public composite_cell {
    pp_jst_fn      m_fn;
    optional<expr> m_expr;
    ext_composite_cell(justification const & j1, justification const & j2, pp_jst_fn const & fn, optional<expr> const & e):
        composite_cell(justification_kind::ExtComposite, j1, j2),
        m_fn(fn),
        m_expr(e) {}
};

struct assumption_cell : public justification_cell {
    unsigned m_idx;
    assumption_cell(justification_kind k, unsigned idx):
        justification_cell(k), m_idx(idx) {}
    assumption_cell(unsigned idx):
        assumption_cell(justification_kind::Assumption, idx) {}
};

struct ext_assumption_cell : public assumption_cell {
    pp_jst_fn      m_fn;
    optional<expr> m_expr;
    ext_assumption_cell(unsigned idx, pp_jst_fn const & fn, optional<expr> const & e):
        assumption_cell(justification_kind::ExtAssumption, idx),
        m_fn(fn),
        m_expr(e) {}
};

asserted_cell * to_asserted(justification_cell * j) { lean_assert(j && j->is_asserted()); return static_cast<asserted_cell*>(j); }
assumption_cell * to_assumption(justification_cell * j) { lean_assert(j && j->is_assumption()); return static_cast<assumption_cell*>(j); }
ext_assumption_cell * to_ext_assumption(justification_cell * j) { lean_assert(j && j->is_ext_assumption()); return static_cast<ext_assumption_cell*>(j); }
composite_cell * to_composite(justification_cell * j) { lean_assert(j && j->is_composite()); return static_cast<composite_cell*>(j); }
ext_composite_cell * to_ext_composite(justification_cell * j) { lean_assert(j && j->is_composite()); return static_cast<ext_composite_cell*>(j); }

approx_set get_approx_assumption_set(justification const & j) {
    justification_cell * it = j.raw();
    if (!it)
        return mk_empty_set();
    switch (it->m_kind) {
    case justification_kind::Asserted:
        return mk_empty_set();
    case justification_kind::Assumption: case justification_kind::ExtAssumption:
        return mk_singleton(to_assumption(it)->m_idx);
    case justification_kind::Composite: case justification_kind::ExtComposite:
        return to_composite(it)->m_assumption_set;
    }
    lean_unreachable(); // LCOV_EXCL_LINE
}

typedef memory_pool<sizeof(asserted_cell)>        asserted_allocator;
typedef memory_pool<sizeof(composite_cell)>       composite_allocator;
typedef memory_pool<sizeof(ext_composite_cell)>   ext_composite_allocator;
typedef memory_pool<sizeof(assumption_cell)>      assumption_allocator;
typedef memory_pool<sizeof(ext_assumption_cell)>  ext_assumption_allocator;
MK_THREAD_LOCAL_GET_DEF(asserted_allocator,       get_asserted_allocator);
MK_THREAD_LOCAL_GET_DEF(composite_allocator,      get_composite_allocator);
MK_THREAD_LOCAL_GET_DEF(ext_composite_allocator,  get_ext_composite_allocator);
MK_THREAD_LOCAL_GET_DEF(assumption_allocator,     get_assumption_allocator);
MK_THREAD_LOCAL_GET_DEF(ext_assumption_allocator, get_ext_assumption_allocator);

void justification_cell::dealloc() {
    switch (m_kind) {
    case justification_kind::Asserted:
        to_asserted(this)->~asserted_cell();
        get_asserted_allocator().recycle(this);
        break;
    case justification_kind::Assumption:
        to_assumption(this)->~assumption_cell();
        get_assumption_allocator().recycle(this);
        break;
    case justification_kind::ExtAssumption:
        to_ext_assumption(this)->~ext_assumption_cell();
        get_ext_assumption_allocator().recycle(this);
        break;
    case justification_kind::Composite:
        to_composite(this)->~composite_cell();
        get_composite_allocator().recycle(this);
        break;
    case justification_kind::ExtComposite:
        to_ext_composite(this)->~ext_composite_cell();
        get_ext_composite_allocator().recycle(this);
        break;
    }
}

struct depends_on_cache {
    unsigned                                m_capacity;
    std::vector<justification_cell const *> m_cache;
    std::vector<unsigned>                   m_used;
    depends_on_cache(unsigned c):m_capacity(c), m_cache(c, nullptr) {}

    bool visited(justification_cell const * j) {
        unsigned i = j->m_hash_alloc % m_capacity;
        if (m_cache[i]) {
            if (m_cache[i] == j) {
                return true;
            } else {
                m_cache[i] = j;
                return false;
            }
        } else {
            m_cache[i] = j;
            m_used.push_back(i);
            return false;
        }
    }

    void clear() {
        for (unsigned i : m_used) {
            lean_assert(m_cache[i]);
            m_cache[i] = nullptr;
        }
        m_used.clear();
    }
};

MK_THREAD_LOCAL_GET(depends_on_cache, get_depends_on_cache, LEAN_DEFAULT_DEPENDS_ON_CACHE_CAPACITY);

struct depends_on_cache_ref {
    depends_on_cache & m_cache;
    depends_on_cache_ref():m_cache(get_depends_on_cache()) {}
    ~depends_on_cache_ref() { m_cache.clear(); }
    depends_on_cache * operator->() { return &m_cache; }
};

bool depends_on(justification const & j, unsigned i) {
    if (!may_contain(get_approx_assumption_set(j), i))
        return false;
    depends_on_cache_ref cache;
    buffer<justification_cell *> todo;
    todo.push_back(j.raw());
    while (!todo.empty()) {
        justification_cell * curr = todo.back();
        todo.pop_back();
        switch (curr->m_kind) {
        case justification_kind::Asserted:
            break;
        case justification_kind::Assumption: case justification_kind::ExtAssumption:
            if (to_assumption(curr)->m_idx == i)
                return true;
            break;
        case justification_kind::Composite: case justification_kind::ExtComposite:
            for (unsigned k = 0; k < 2; k++) {
                justification c = to_composite(curr)->m_child[k];
                if (!cache->visited(c.raw()) && may_contain(get_approx_assumption_set(c), i))
                    todo.push_back(c.raw());
            }
        }
    }
    return false;
}

justification const & composite_child1(justification const & j) {
    lean_assert(j.is_composite());
    return to_composite(j.raw())->m_child[0];
}

justification const & composite_child2(justification const & j) {
    lean_assert(j.is_composite());
    return to_composite(j.raw())->m_child[1];
}

unsigned assumption_idx(justification const & j) {
    lean_assert(j.is_assumption());
    return to_assumption(j.raw())->m_idx;
}

justification::justification():m_ptr(nullptr) {}
justification::justification(justification_cell * ptr):m_ptr(ptr) { if (m_ptr) m_ptr->inc_ref(); }
justification::justification(justification const & s):m_ptr(s.m_ptr) { if (m_ptr) m_ptr->inc_ref(); }
justification::justification(justification && s):m_ptr(s.m_ptr) { s.m_ptr = nullptr; }
justification::~justification() { if (m_ptr) m_ptr->dec_ref(); }
bool justification::is_none() const { return m_ptr == nullptr; }
bool justification::is_asserted() const { return m_ptr && m_ptr->is_asserted(); }
bool justification::is_assumption() const { return m_ptr && m_ptr->is_assumption(); }
bool justification::is_composite() const { return m_ptr && m_ptr->is_composite(); }
justification & justification::operator=(justification const & s) { LEAN_COPY_REF(s); }
justification & justification::operator=(justification && s) { LEAN_MOVE_REF(s); }
optional<expr> justification::get_main_expr() const {
    justification_cell * it = m_ptr;
    while (true) {
        if (!it)
            return none_expr();
        switch (it->m_kind) {
        case justification_kind::Asserted:
            return to_asserted(it)->m_expr;
        case justification_kind::ExtAssumption:
            return to_ext_assumption(it)->m_expr;
        case justification_kind::ExtComposite:
            return to_ext_composite(it)->m_expr;
        case justification_kind::Assumption:
            return none_expr();
        case justification_kind::Composite:
            it = to_composite(it)->m_child[0].raw();
            break;
        }
    }
}
format justification::pp(formatter const & fmt, pos_info_provider const * p, substitution const & s) const {
    justification_cell * it = m_ptr;
    while (true) {
        if (!it)
            return format();
        switch (it->m_kind) {
        case justification_kind::Asserted:
            return to_asserted(it)->m_fn(fmt, p, s);
        case justification_kind::ExtAssumption:
            return to_ext_assumption(it)->m_fn(fmt, p, s);
        case justification_kind::ExtComposite:
            return to_ext_composite(it)->m_fn(fmt, p, s);
        case justification_kind::Assumption:
            return format(format("Assumption "), format(to_assumption(it)->m_idx));
        case justification_kind::Composite:
            it = to_composite(it)->m_child[0].raw();
            break;
        }
    }
}

justification mk_composite(justification const & j1, justification const & j2, optional<expr> const & s, pp_jst_fn const & fn) {
    if (j1.is_none())
        return j2;
    if (j2.is_none())
        return j1;
    return justification(new (get_ext_composite_allocator().allocate()) ext_composite_cell(j1, j2, fn, s));
}
justification mk_composite1(justification const & j1, justification const & j2) {
    if (j1.is_none())
        return j2;
    if (j2.is_none())
        return j1;
    return justification(new (get_composite_allocator().allocate()) composite_cell(j1, j2));
}
justification mk_assumption_justification(unsigned idx, optional<expr> const & s, pp_jst_fn const & fn) {
    return justification(new (get_ext_assumption_allocator().allocate()) ext_assumption_cell(idx, fn, s));
}
justification mk_assumption_justification(unsigned idx) {
    return justification(new (get_assumption_allocator().allocate()) assumption_cell(idx));
}
justification mk_justification(optional<expr> const & s, pp_jst_fn const & fn) {
    return justification(new (get_asserted_allocator().allocate()) asserted_cell(fn, s));
}
justification mk_justification(optional<expr> const & s, pp_jst_sfn const & fn) {
    return mk_justification(s, [=](formatter const & fmt, pos_info_provider const *, substitution const & subst) {
            // Remark: we are not using to_pos(s, p) anymore because we don't try to display complicated error messages anymore.
            // return compose(to_pos(s, p), fn(fmt, subst));
            return fn(fmt, subst);
        });
}
justification mk_justification(char const * msg, optional<expr> const & s) {
    std::string _msg(msg);
    return mk_justification(s, [=](formatter const &, pos_info_provider const *, substitution const &) {
            return format(_msg);
        });
}
std::ostream & operator<<(std::ostream & out, justification const & j) {
    if (j.is_none()) {
        out << "none";
    } else if (j.is_asserted()) {
        out << "asserted";
    } else if (j.is_assumption()) {
        out << "(assumption " << assumption_idx(j) << ")";
    } else if (j.is_composite()) {
        out << "(join " << composite_child1(j) << " " << composite_child2(j) << ")";
    } else {
        out << "unexpected";
    }
    return out;
}
}
