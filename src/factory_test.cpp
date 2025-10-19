#include "../include/Socket.hpp"
#include <iostream>

int main()
{
    std::cout << "Testing UDPSocket factory method..." << std::endl;

    // Test successful socket creation
    auto socket_result = r3::UDPSocket::create();
    if (socket_result)
    {
        std::cout << "✓ Socket created successfully" << std::endl;
        std::cout << "✓ Socket is valid: " << (socket_result.value().is_valid() ? "yes" : "no") << std::endl;
    }
    else
    {
        std::cout << "✗ Failed to create socket: " << r3::to_string(socket_result.error()) << std::endl;
    }

    // Test multiple socket creation
    std::cout << "\nCreating multiple sockets..." << std::endl;
    for (int i = 0; i < 3; ++i)
    {
        auto test_socket = r3::UDPSocket::create();
        if (test_socket)
        {
            std::cout << "✓ Socket " << i + 1 << " created successfully" << std::endl;
        }
        else
        {
            std::cout << "✗ Socket " << i + 1 << " creation failed: " << r3::to_string(test_socket.error()) << std::endl;
        }
    }

    return 0;
}