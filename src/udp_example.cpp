#include "../include/Socket.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

// Simple UDP Server example
void udp_server_example()
{

    auto server_result = r3::UDPSocket::create();
    if (!server_result)
    {
        std::cerr << "Failed to create server socket: " << r3::to_string(server_result.error()) << std::endl;
        return;
    }

    auto &server = server_result.value();

    // Set socket options with error handling
    auto reuse_result = server.set_reuse_address(true);
    if (reuse_result)
    {
        std::cout << "✓ Socket reuse address enabled" << std::endl;
    }
    else
    {
        std::cerr << "✗ Failed to set reuse address: " << r3::to_string(reuse_result.error()) << std::endl;
    }

    // Set timeout with error handling
    auto timeout_result = server.set_timeout(5, 0); // 5 seconds timeout
    if (timeout_result)
    {
        std::cout << "✓ Socket timeout set to 5 seconds" << std::endl;
    }
    else
    {
        std::cerr << "✗ Failed to set timeout: " << r3::to_string(timeout_result.error()) << std::endl;
    }

    // Bind to port 8080 on all interfaces
    auto bind_result = server.bind(r3::kAny, 8080);
    if (!bind_result)
    {
        std::cerr << "Failed to bind server socket: " << r3::to_string(bind_result.error()) << std::endl;
        return;
    }

    std::cout << "UDP Server listening on port 8080..." << std::endl;

    // Demonstrate getting local address info
    auto local_addr = server.get_local_address();
    auto local_port = server.get_local_port();

    if (local_addr && local_port)
    {
        std::cout << "Server bound to: " << local_addr.value().to_string()
                  << ":" << local_port.value() << std::endl;
    }
    else
    {
        std::cerr << "Warning: Could not retrieve local address info" << std::endl;
    }

    char buffer[1024];

    while (true)
    {
        auto receive_result = server.receive_from(buffer, sizeof(buffer) - 1);

        if (receive_result)
        {
            auto &result = receive_result.value();
            buffer[result.bytes_received] = '\0';
            std::cout << "Received from " << result.sender_address.to_string() << ":" << result.sender_port
                      << " -> " << buffer << std::endl;

            // Echo back
            std::string response = "Echo: " + std::string(buffer);
            auto send_result = server.send_to(response.c_str(), response.length(), result.sender_address, result.sender_port);
            if (!send_result)
            {
                std::cerr << "Failed to send response: " << r3::to_string(send_result.error()) << std::endl;
            }
        }
        else
        {
            std::cerr << "Failed to receive data: " << r3::to_string(receive_result.error()) << std::endl;
        }
    }
}

// Simple UDP Client example
void udp_client_example()
{

    auto client_result = r3::UDPSocket::create();
    if (!client_result)
    {
        std::cerr << "Failed to create client socket: " << r3::to_string(client_result.error()) << std::endl;
        return;
    }

    auto &client = client_result.value();

    // Set a timeout for the client as well
    auto timeout_result = client.set_timeout(3, 0); // 3 seconds timeout
    if (!timeout_result)
    {
        std::cerr << "Warning: Failed to set client timeout: " << r3::to_string(timeout_result.error()) << std::endl;
    }

    std::string message = "Hello UDP Server!";

    std::cout << "Sending message to server..." << std::endl;

    // Send message to server
    auto send_result = client.send_to(message.c_str(), message.length(), r3::kLocalHost, 8080);

    if (send_result)
    {
        std::cout << "Sent: " << message << " (" << send_result.value() << " bytes)" << std::endl;

        // Receive response
        char buffer[1024];
        auto receive_result = client.receive_from(buffer, sizeof(buffer) - 1);

        if (receive_result)
        {
            auto &result = receive_result.value();
            buffer[result.bytes_received] = '\0';
            std::cout << "Received from server (" << result.sender_address.to_string()
                      << ":" << result.sender_port << "): " << buffer << std::endl;
        }
        else
        {
            std::cerr << "Failed to receive response: " << r3::to_string(receive_result.error()) << std::endl;
        }
    }
    else
    {
        std::cerr << "Failed to send message: " << r3::to_string(send_result.error()) << std::endl;
    }
}

int main()
{
    std::cout << "UDP Socket Wrapper Example - Updated for std::expected API" << std::endl;
    std::cout << "============================================================" << std::endl;

    // Demonstrate IPv4Address usage
    std::cout << "\nIPv4Address Constants:" << std::endl;
    std::cout << "LOCALHOST: " << r3::kLocalHost.to_string() << std::endl;
    std::cout << "ANY: " << r3::kAny.to_string() << std::endl;
    std::cout << "BROADCAST: " << r3::kBroadcast.to_string() << std::endl;

    // Demonstrate creating custom IPv4 addresses with error handling
    std::cout << "\nCustom IPv4 Addresses (with std::expected):" << std::endl;

    // Test valid address
    auto addr_result1 = r3::IPv4Address::from_string("192.168.1.100");
    if (addr_result1)
    {
        std::cout << "✓ Valid address from string: " << addr_result1.value().to_string() << std::endl;
    }
    else
    {
        std::cerr << "✗ Failed to parse address: " << r3::to_string(addr_result1.error()) << std::endl;
    }

    // Test invalid address to demonstrate error handling
    auto addr_result2 = r3::IPv4Address::from_string("invalid.address");
    if (addr_result2)
    {
        std::cout << "✓ Parsed address: " << addr_result2.value().to_string() << std::endl;
    }
    else
    {
        std::cout << "✓ Expected error for invalid address: " << r3::to_string(addr_result2.error()) << std::endl;
    }

    // Test constexpr constructor
    r3::IPv4Address custom3(192, 168, 1, 1);
    std::cout << "✓ From octets: " << custom3.to_string() << std::endl;

    std::cout << "\n=== UDP Socket Communication Test ===" << std::endl;
    std::cout << "1. Starting server in background" << std::endl;
    std::cout << "2. Running client" << std::endl;

    // Start server in a separate thread
    std::thread server_thread(udp_server_example);
    server_thread.detach();

    // Wait a moment for server to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Run client
    udp_client_example();

    // Keep main thread alive for a bit to see server output
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "\n=== Testing Broadcast Functionality ===" << std::endl;

    // Demonstrate broadcast capability
    try
    {
        auto broadcast_result = r3::UDPSocket::create();
        if (!broadcast_result)
        {
            std::cerr << "Failed to create broadcast socket: " << r3::to_string(broadcast_result.error()) << std::endl;
        }
        else
        {
            auto &broadcast_socket = broadcast_result.value();
            auto broadcast_enable_result = broadcast_socket.set_broadcast(true);
            if (broadcast_enable_result)
            {
                std::cout << "✓ Broadcast enabled successfully" << std::endl;

                // Try to send a broadcast message (this might fail if no broadcast route exists)
                std::string broadcast_msg = "Broadcast test message";
                auto send_result = broadcast_socket.send_to(broadcast_msg.c_str(), broadcast_msg.length(),
                                                            r3::kBroadcast, 9999);
                if (send_result)
                {
                    std::cout << "✓ Broadcast message sent (" << send_result.value() << " bytes)" << std::endl;
                }
                else
                {
                    std::cout << "✗ Broadcast send failed (expected on some networks): "
                              << r3::to_string(send_result.error()) << std::endl;
                }
            }
            else
            {
                std::cout << "✗ Failed to enable broadcast: " << r3::to_string(broadcast_enable_result.error()) << std::endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Broadcast test error: " << e.what() << std::endl;
    }

    return 0;
}