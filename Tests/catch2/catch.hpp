/*
  ==============================================================================

    catch.hpp
    Minimal Catch2 v2.x single-header implementation
    
    This is a simplified version for property-based testing.
    For full Catch2 features, use the official library.

  ==============================================================================
*/

#ifndef CATCH_HPP_INCLUDED
#define CATCH_HPP_INCLUDED

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <exception>
#include <cmath>

namespace Catch {

// Test registry
struct TestCase {
    std::string name;
    std::string tags;
    void (*func)();
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry registry;
        return registry;
    }
    
    void registerTest(const std::string& name, const std::string& tags, void (*func)()) {
        tests.push_back({name, tags, func});
    }
    
    int runAllTests() {
        int passed = 0;
        int failed = 0;
        
        std::cout << "\n===============================================================================\n";
        std::cout << "Running " << tests.size() << " test cases...\n";
        std::cout << "===============================================================================\n\n";
        
        for (const auto& test : tests) {
            std::cout << "-------------------------------------------------------------------------------\n";
            std::cout << test.name << "\n";
            std::cout << test.tags << "\n";
            std::cout << "-------------------------------------------------------------------------------\n";
            
            try {
                currentTestName = test.name;
                infoMessages.clear();
                test.func();
                std::cout << "PASSED\n\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "FAILED:\n";
                for (const auto& info : infoMessages) {
                    std::cout << "  " << info << "\n";
                }
                std::cout << "  " << e.what() << "\n\n";
                failed++;
            }
        }
        
        std::cout << "===============================================================================\n";
        std::cout << "Test results: " << passed << " passed, " << failed << " failed\n";
        std::cout << "===============================================================================\n";
        
        return failed;
    }
    
    void addInfo(const std::string& msg) {
        infoMessages.push_back(msg);
    }
    
    std::string getCurrentTestName() const {
        return currentTestName;
    }
    
private:
    std::vector<TestCase> tests;
    std::vector<std::string> infoMessages;
    std::string currentTestName;
};

// Test registration helper
struct TestRegistrar {
    TestRegistrar(const std::string& name, const std::string& tags, void (*func)()) {
        TestRegistry::instance().registerTest(name, tags, func);
    }
};

// Assertion exception
class AssertionException : public std::exception {
public:
    AssertionException(const std::string& msg, const char* file, int line) {
        std::ostringstream oss;
        oss << file << ":" << line << ": " << msg;
        message = oss.str();
    }
    
    const char* what() const noexcept override {
        return message.c_str();
    }
    
private:
    std::string message;
};

// Info message helper
class InfoCapture {
public:
    InfoCapture(const std::string& msg) {
        TestRegistry::instance().addInfo(msg);
    }
};

} // namespace Catch

// Macros
#define CATCH_INTERNAL_LINEINFO2(name, line) name##line
#define CATCH_INTERNAL_LINEINFO(name, line) CATCH_INTERNAL_LINEINFO2(name, line)

#define TEST_CASE(name, tags) \
    static void CATCH_INTERNAL_LINEINFO(test_, __LINE__)(); \
    static Catch::TestRegistrar CATCH_INTERNAL_LINEINFO(registrar_, __LINE__)(name, tags, CATCH_INTERNAL_LINEINFO(test_, __LINE__)); \
    static void CATCH_INTERNAL_LINEINFO(test_, __LINE__)()

#define REQUIRE(expr) \
    do { \
        if (!(expr)) { \
            throw Catch::AssertionException("REQUIRE( " #expr " ) failed", __FILE__, __LINE__); \
        } \
    } while(0)

#define REQUIRE_NOTHROW(expr) \
    do { \
        try { \
            expr; \
        } catch (...) { \
            throw Catch::AssertionException("REQUIRE_NOTHROW( " #expr " ) failed - exception thrown", __FILE__, __LINE__); \
        } \
    } while(0)

#define INFO(msg) \
    do { \
        std::ostringstream oss; \
        oss << msg; \
        Catch::InfoCapture info_##__LINE__(oss.str()); \
    } while(0)

#define SUCCEED(msg) \
    do { \
        std::cout << "  " << msg << "\n"; \
    } while(0)

#ifdef CATCH_CONFIG_MAIN
int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return Catch::TestRegistry::instance().runAllTests();
}
#endif

#endif // CATCH_HPP_INCLUDED
