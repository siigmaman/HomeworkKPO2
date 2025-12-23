#define CATCH_CONFIG_MAIN
#include "../third_party/catch.hpp"
#include "../include/domain.hpp"
#include "../include/factory.hpp"
#include "../include/repository.hpp"
#include <memory>

TEST_CASE("Domain factory produces valid objects") {
    DomainFactory f;
    auto a = f.createAccount("Test", 100.0);
    REQUIRE(a.id > 0);
    REQUIRE(a.name == "Test");
    REQUIRE(a.balance == 100.0);
}

TEST_CASE("BankAccount operations work correctly") {
    auto acc = DomainFactory().createAccount("Test", 100.0);
    
    SECTION("Balance access") {
        REQUIRE(acc.balance == 100.0);
    }
    
    SECTION("Account properties") {
        REQUIRE(acc.name == "Test");
        REQUIRE(acc.id > 0);
    }
}

TEST_CASE("Factory generates unique IDs") {
    DomainFactory f;
    auto a1 = f.createAccount("A", 100.0);
    auto a2 = f.createAccount("B", 200.0);
    auto a3 = f.createAccount("C", 300.0);
    
    REQUIRE(a1.id != a2.id);
    REQUIRE(a2.id != a3.id);
    REQUIRE(a3.id != a1.id);
}

TEST_CASE("Account validation") {
    DomainFactory f;
    
    SECTION("Valid account creation") {
        REQUIRE_NOTHROW(f.createAccount("Valid", 100.0));
    }
    
    SECTION("Invalid account creation throws") {
        REQUIRE_THROWS(f.createAccount("", 100.0)); 
        REQUIRE_THROWS(f.createAccount("Test", -50.0)); 
    }
}
