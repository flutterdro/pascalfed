#ifndef FED_NAME_SCOPE_HPP_AJNAODCNL
#define FED_NAME_SCOPE_HPP_AJNAODCNL

#include <string>
#include <unordered_map>

namespace fed::ast {

// glue class. ast doesn't work with stringy names.
// it hashes a name and transforms it into index in the
// symbol table
enum class symbol_kind : unsigned {
    type,
    variable,
    constant,
    function,
    procedure,
};
struct tagged_id {
    unsigned id;
    symbol_kind kind;
};
class name_scope {
    struct string_hash {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;
        constexpr auto operator()(std::string const& str) const
            -> std::size_t { return hash_type()(str); }
        constexpr auto operator()(std::string_view const& str) const
            -> std::size_t { return hash_type()(str); }
        constexpr auto operator()(char const* str) const
            -> std::size_t { return hash_type()(str); }
    };
    using map_t = std::unordered_map<
        std::string, tagged_id, 
        string_hash, std::equal_to<>
    >; 
    using iterator = map_t::iterator;
    struct insert_result {
        bool is_success;
        iterator insertee;
    };
public:
    constexpr name_scope() noexcept 
        : m_scope(), m_parent(nullptr) {}
    name_scope(name_scope&&) = delete;
    name_scope(name_scope const&) = delete;
    auto operator=(name_scope&&) 
        -> name_scope& = delete;
    auto operator=(name_scope const&)
        -> name_scope& = delete;
    name_scope(name_scope const&, map_t) noexcept;
    auto spawn() const
        -> std::unique_ptr<name_scope>;
    auto insert(std::string, tagged_id) 
        -> insert_result;
    auto lookup(std::string_view) const noexcept
        -> std::optional<tagged_id>;
private:
    map_t             m_scope;
    name_scope const* m_parent;
};

}


#endif
