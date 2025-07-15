#include <catch2/catch_test_macros.hpp>

#include "fed/parser/context.hpp"
#include "fed/representations/ast.hpp"

#include "commons.hpp"
#include "fed/utils/superutil.hpp"

SCENARIO("Symbol table insertion and scoping", "[frontend][semantics]") {
    GIVEN("globale scope") {
        auto global = fed::semantic_context::make_global();
        WHEN("add names to it") {
            auto err1 = global.add_type({
                .name = "at",
                .type = fed::poison_pill,
            });
            auto err2 = global.add_constant({
                .name = "ac",
                .constant = fed::poison_pill,
            });
            auto err3 = global.add_variable({
                .name = "av",
                .type = fed::poison_pill,
            });
            THEN("no problem with insertion") {
                CHECK(err1);
                CHECK(err2);
                CHECK(err3);
            }
            THEN("they should be present") {
                auto const tid = global.try_get_type_id("at");
                auto const cid = global.try_get_constant_id("ac");
                auto const vid = global.try_get_variable_id("av");
                REQUIRE(tid);
                REQUIRE(cid);
                REQUIRE(vid);
            }
            AND_WHEN("creating subscope") {
                auto local = global.make_local();
                THEN("they still should be present") {
                    auto const tid = local.try_get_type_id("at");
                    auto const cid = local.try_get_constant_id("ac");
                    auto const vid = local.try_get_variable_id("av");
                    REQUIRE(tid);
                    REQUIRE(cid);
                    REQUIRE(vid);
                }
                AND_WHEN("adding new variable to local") {
                    auto err = local.add_variable({
                        .name = "bv",
                        .type = fed::poison_pill,
                    });
                    CHECK(err);
                    THEN("it should be present") {
                        auto const vid = local.try_get_variable_id("bv");
                        REQUIRE(vid);
                        AND_THEN("global shouldn't be affected") {
                            auto const vid = global.try_get_variable_id("bv");
                            REQUIRE(not vid);
                        }
                    }
                }
            }
        }
    }
    
}
