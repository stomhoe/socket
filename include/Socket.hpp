
#include <array>
#include <cstdint>
#include <string>
#include <expected>
#include <system_error>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#pragma once

namespace r3
{
    enum class SocketError
    {
        kInvalidSocket,
        kBindFailed,
        kConnectFailed,
        kSendFailed,
        kReceiveFailed,
        kInvalidAddress,
        kSocketOptionFailed,
        kNotBound,
        kAddressParseError,
        kTrainOrderParseError
    };

    class TrainOrder
    {
    private:
        enum Action
        {
            kAccelerate,
            kMaintainSpeed,
            kBrake
        };
        Action m_action;

    public:
        // Constructors
        constexpr TrainOrder() : m_action(kMaintainSpeed) {}
        constexpr explicit TrainOrder(Action action) : m_action(action) {}
        explicit TrainOrder(const std::string &action_str);

        // Factory methods
        static constexpr TrainOrder accelerate() { return TrainOrder(kAccelerate); }
        static constexpr TrainOrder maintain_speed() { return TrainOrder(kMaintainSpeed); }
        static constexpr TrainOrder brake() { return TrainOrder(kBrake); }

        bool is_accelerate() const { return m_action == kAccelerate; }
        bool is_maintain_speed() const { return m_action == kMaintainSpeed; }
        bool is_brake() const { return m_action == kBrake; }

        // Getters
        constexpr Action get_action() const { return m_action; }

        // Operators
        constexpr bool operator==(const TrainOrder &other) const { return m_action == other.m_action; }
        constexpr bool operator!=(const TrainOrder &other) const { return m_action != other.m_action; }

        // Conversion to string
        std::string to_string() const;
    };

    class TrainVisualizationData
    {
    public:
        TrainVisualizationData(float position, float speed, float acc_time)
            : position_(position), speed_(speed), acc_time_(acc_time) {}
        const float position_;
        const float speed_;
        const float acc_time_;
    };

    // Convert error code to string
    std::string to_string(SocketError error);
    // IPv4 address type
    struct IPv4Address
    {
        std::array<uint8_t, 4> octets;

        // Constructors
        constexpr IPv4Address() : octets{0, 0, 0, 0} {}
        constexpr IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d) : octets{a, b, c, d} {}
        explicit IPv4Address(uint32_t addr);

        // Static factory method for string parsing
        static std::expected<IPv4Address, SocketError> from_string(const std::string &addr_str);

        // Convert to string representation
        std::string to_string() const;

        // Convert to network byte order uint32_t
        uint32_t to_uint32() const;

        // Operators
        bool operator==(const IPv4Address &other) const;
        bool operator!=(const IPv4Address &other) const;
    };

    constexpr IPv4Address kLocalHost{127, 0, 0, 1};
    constexpr IPv4Address kAny{0, 0, 0, 0};
    constexpr IPv4Address kBroadcast{255, 255, 255, 255};

    // UDP Socket Wrapper Class
    class UDPSocket
    {
    private:
        int m_socket_fd;
        struct sockaddr_in m_local_addr;
        struct sockaddr_in m_remote_addr;
        bool m_is_bound;

        // Private constructor
        UDPSocket();

    public:
        // Static factory method for creating sockets
        static std::expected<UDPSocket, SocketError> create();

        ~UDPSocket();

        // Copy operations
        UDPSocket(const UDPSocket &) = delete;
        UDPSocket &operator=(const UDPSocket &) = delete;

        // Move operations
        UDPSocket(UDPSocket &&other) noexcept;
        UDPSocket &operator=(UDPSocket &&other) noexcept;

        // Socket operations
        std::expected<void, SocketError> bind(const IPv4Address &address, uint16_t port);
        // Data transmission
        std::expected<ssize_t, SocketError> send_order_to(const TrainOrder order, const IPv4Address &address, uint16_t port);
        std::expected<ssize_t, SocketError> send_visualization_data_to(const TrainVisualizationData data, const IPv4Address &address, uint16_t port);
        std::expected<ssize_t, SocketError> send_to(const void *data, size_t size, const IPv4Address &address, uint16_t port);
        struct ReceiveVisualizationDataResult
        {
            const TrainVisualizationData data;
            const IPv4Address sender_address;

            // Add a constructor for easy initialization
            ReceiveVisualizationDataResult(const TrainVisualizationData &d, const IPv4Address &addr)
                : data(d), sender_address(addr) {}
        };

        struct ReceiveOrderResult
        {
            TrainOrder order;
            IPv4Address sender_address;
        };

        struct ReceiveFromResult
        {
            ssize_t bytes_received;
            IPv4Address sender_address;
            uint16_t sender_port;
        };
        std::expected<ReceiveFromResult, SocketError> receive_from(void *buffer, size_t size);
        std::expected<ReceiveOrderResult, SocketError> receive_order_from(TrainOrder &order);
        std::expected<UDPSocket::ReceiveVisualizationDataResult, SocketError> receive_visualization_data(TrainVisualizationData &data);

        // Socket management
        void close();
        bool is_valid() const;
        int get_fd() const;

        // Address utilities
        std::expected<IPv4Address, SocketError> get_local_address() const;
        std::expected<uint16_t, SocketError> get_local_port() const;
        std::expected<IPv4Address, SocketError> get_remote_address() const;
        std::expected<uint16_t, SocketError> get_remote_port() const;

        // Socket options
        std::expected<void, SocketError> set_broadcast(bool enable);
        std::expected<void, SocketError> set_reuse_address(bool enable);
        std::expected<void, SocketError> set_timeout(int seconds, int microseconds = 0);
    };
} // namespace r3