#ifndef FED_SYMBOL_TABLE_HPP_
#define FED_SYMBOL_TABLE_HPP_

#include "fed/diagnostics/compile-error.hpp"
#include "fed/diagnostics/internal-error.hpp"
#include "fed/representations/raw-source.hpp"
#include <string_view>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <variant>

namespace fed {

namespace ast {
    struct type_definition;
    struct constant_definition;
    struct variable_declaration;
    struct function_declaration;
}

namespace sym {
enum class name_type {
    type,
    variable,
    constant,
    function,
};
struct poison_t {};
struct type {
    enum class id : unsigned { null = 0, poison, };
    using info = ast::type_definition*;
};
struct constant {
    enum class id : unsigned { null = 0, };
    using info = std::variant<int, char, double, std::string_view, poison_t>;
};
struct function {
    enum class id : unsigned { null = 0, };
    using info = ast::function_declaration*;
};
struct variable {
    enum class id : unsigned { null = 0, };
    using info = ast::variable_declaration*;
};

struct table_error : fed::compilation_error {
    enum class type {
        lookup_not_found = 1,
        lookup_found_wrong,
        insert_already_exists,
    };
    source::view m_area;
};

template<typename Symbol>
class base_table {
public:
    using id        = typename Symbol::id;
    using info      = typename Symbol::info;
    using container = std::vector<info>;
    using iterator  = typename container::const_iterator;

    base_table()
        : m_data(1) {}

    auto lookup(id sid) const
        -> info { 
        if (sid == id::null) throw fed::internal_error("lookup of id::null");
        auto const key = std::to_underlying(sid);
        if (key >= m_data.size()) {
            throw fed::internal_error("lookup of unknown id");
        } else {
            return m_data[key];
        }
    }
    auto insert(info info)
        -> id {
        m_data.push_back(info);
        return static_cast<id>(m_data.size() - 1);
    }
private:
    container m_data;
};


class scope {
    struct transparent_hasher {
        using hash_base = std::hash<std::string_view>;
        using is_transperent = void;

        auto operator()(std::string_view str) const 
            -> std::size_t { return hash_base{}(str); }
        auto operator()(std::string const& str) const 
            -> std::size_t { return hash_base{}(str); }
        auto operator()(char const* str) const 
            -> std::size_t { return hash_base{}(str); }
    };
public:
    struct name_id {
        name_type type;
        unsigned id;
    };
    using name_map = std::unordered_map<std::string_view, name_id, transparent_hasher, std::equal_to<>>;
public:
    using iterator = name_map::iterator;
public:
    auto insert(std::string_view name, name_id id)
         { return m_names.insert({name, id}); }
    auto lookup(std::string_view name) 
        -> iterator { return m_names.find(name); }
    auto end()
        -> iterator { return m_names.end(); }
private:
    name_map m_names;
};

class scope_tree {
public:
    class walker {
    public:
        walker(scope_tree* base) 
            : m_base(base) {}
        auto advance_to_child(std::size_t index) 
            -> walker { 
            m_base = m_base->m_kids[index].get();
            return *this;
        }
        auto backtrack_to_parent()
            -> walker { 
            m_base = m_base->m_parent;
            return *this;
        }
        auto operator==(walker const&) const -> bool = default;
        auto base() 
            -> scope_tree* { return m_base; }
        auto base() const
            -> scope_tree const* { return m_base; }
        auto scope() 
            -> scope& { return m_base->m_names; }
    private:
        scope_tree* m_base;
    };
public:
    scope_tree(scope_tree const&) = delete;
    scope_tree(scope_tree&&) = default;
    auto concieve_at(walker walk)
        -> walker { return walk.base()->birth_child(); }
    auto get_walker()
        -> walker { return {this}; }
private:
    scope_tree(scope_tree* parent)
        : m_parent(parent) {}
    auto birth_child()
        -> walker { 
        m_kids.push_back(std::make_unique<scope_tree>(scope_tree(this)));
        return walker(m_kids.back().get());
    }
private:
    scope_tree* m_parent;
    scope       m_names;
    std::vector<std::unique_ptr<scope_tree>> m_kids;
};

class table {
public: 
    auto types() 
        -> base_table<type>& { return m_types; }
    auto constants()
        -> base_table<constant>& { return m_constants; }
    auto variables()
        -> base_table<variable>& { return m_variables; }
    auto functions()
        -> base_table<function>& { return m_functions; }

    auto types() const
        -> base_table<type> const& { return m_types; }
    auto constants() const
        -> base_table<constant> const& { return m_constants; }
    auto variables() const
        -> base_table<variable> const& { return m_variables; }
    auto functions() const
        -> base_table<function> const& { return m_functions; }

    base_table<type> m_types;
    base_table<constant> m_constants;
    base_table<variable> m_variables;
    base_table<function> m_functions;
};
}
} // namespace fed

#endif
