#include <gtest/gtest.h>
#include <fstream>
#include "CGIHandler.hpp"
#include "Request.hpp"
#include "Response.hpp"

// A sample CGI script for testing purposes.
// (Raw string literals(C++11))
const std::string TEST_SCRIPT = R"(
#!/bin/bash
echo "Status: 200 OK"
echo "Content-Type: text/plain"
echo ""
echo "Hello, World!"
)";

// Write the test script to a temporary file
std::string create_test_script() {
    std::string script_path = "/tmp/test_cgi_script.sh";
    std::ofstream script_file(script_path);
    script_file << TEST_SCRIPT;
    script_file.close();
    chmod(script_path.c_str(), 0755); // Make the script executable
    return script_path;
}

// Test fixture for CGIHandler
class CGIHandlerTest : public ::testing::Test {
protected:
    std::string script_path;

    void SetUp() override {
        script_path = create_test_script();
    }

    void TearDown() override {
        remove(script_path.c_str());
    }
};

// Test case for a normal GET request
TEST_F(CGIHandlerTest, ExecuteGETRequest) {
    CGIHandler handler(script_path);

    // Simulate a simple GET request
    std::string raw_request = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    Request request(raw_request);
		std::cout << "DEBUG" << std::endl;
		std::cout << request.get_method() << std::endl;
		std::cout << request.get_path() << std::endl;
		std::cout << request.get_headers()["Host"] << std::endl;

    Response response = handler.execute(request);

    EXPECT_EQ(response.get_status_code(), 200);
    EXPECT_EQ(response.get_headers()["Content-Type"], "text/plain");
    EXPECT_EQ(response.get_body(), "Hello, World!\n");
}

// Test case for a normal POST request
TEST_F(CGIHandlerTest, ExecutePOSTRequest) {
    CGIHandler handler(script_path);

    // Simulate a simple POST request
    std::string raw_request = "POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 11\r\n\r\nHello World";
    Request request(raw_request);

    Response response = handler.execute(request);

    EXPECT_EQ(response.get_status_code(), 200);
    EXPECT_EQ(response.get_headers()["Content-Type"], "text/plain");
    EXPECT_EQ(response.get_body(), "Hello, World!\n");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
