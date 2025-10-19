#include "../include/Socket.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <sstream>

using namespace r3;

// Error code to string conversion
std::string r3::to_string(SocketError error)
{
    switch (error)
    {
    case SocketError::kInvalidSocket:
        return "Invalid socket";
    case SocketError::kBindFailed:
        return "Bind operation failed";
    case SocketError::kConnectFailed:
        return "Connect operation failed";
    case SocketError::kSendFailed:
        return "Send operation failed";
    case SocketError::kReceiveFailed:
        return "Receive operation failed";
    case SocketError::kInvalidAddress:
        return "Invalid address";
    case SocketError::kSocketOptionFailed:
        return "Socket option operation failed";
    case SocketError::kNotBound:
        return "Socket not bound";
    case SocketError::kAddressParseError:
        return "Address parsing error";
    default:
        return "Unknown error";
    }
}

// IPv4Address Implementation

std::expected<IPv4Address, SocketError> IPv4Address::from_string(const std::string &addr_str)
{
    struct in_addr addr;
    if (inet_pton(AF_INET, addr_str.c_str(), &addr) <= 0)
    {
        return std::unexpected(SocketError::kAddressParseError);
    }

    uint32_t network_addr = addr.s_addr;
    IPv4Address result;
    result.octets[0] = (network_addr >> 0) & 0xFF;
    result.octets[1] = (network_addr >> 8) & 0xFF;
    result.octets[2] = (network_addr >> 16) & 0xFF;
    result.octets[3] = (network_addr >> 24) & 0xFF;
    return result;
}

IPv4Address::IPv4Address(uint32_t addr)
{
    octets[0] = (addr >> 0) & 0xFF;
    octets[1] = (addr >> 8) & 0xFF;
    octets[2] = (addr >> 16) & 0xFF;
    octets[3] = (addr >> 24) & 0xFF;
}

std::string IPv4Address::to_string() const
{
    std::ostringstream oss;
    oss << static_cast<int>(octets[0]) << "."
        << static_cast<int>(octets[1]) << "."
        << static_cast<int>(octets[2]) << "."
        << static_cast<int>(octets[3]);
    return oss.str();
}

uint32_t IPv4Address::to_uint32() const
{
    return (static_cast<uint32_t>(octets[0]) << 0) |
           (static_cast<uint32_t>(octets[1]) << 8) |
           (static_cast<uint32_t>(octets[2]) << 16) |
           (static_cast<uint32_t>(octets[3]) << 24);
}

bool IPv4Address::operator==(const IPv4Address &other) const
{
    return octets == other.octets;
}

bool IPv4Address::operator!=(const IPv4Address &other) const
{
    return !(*this == other);
}

// UDPSocket Implementation

UDPSocket::UDPSocket() : m_socket_fd(-1), m_is_bound(false)
{
    memset(&m_local_addr, 0, sizeof(m_local_addr));
    memset(&m_remote_addr, 0, sizeof(m_remote_addr));

    // Create UDP socket using sys/socket.h
    m_socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
}

std::expected<UDPSocket, SocketError> UDPSocket::create()
{
    UDPSocket socket;
    if (!socket.is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }
    return socket;
}

UDPSocket::~UDPSocket()
{
    close();
}

UDPSocket::UDPSocket(UDPSocket &&other) noexcept
    : m_socket_fd(other.m_socket_fd),
      m_local_addr(other.m_local_addr),
      m_remote_addr(other.m_remote_addr),
      m_is_bound(other.m_is_bound)
{
    other.m_socket_fd = -1;
    other.m_is_bound = false;
}

UDPSocket &UDPSocket::operator=(UDPSocket &&other) noexcept
{
    if (this != &other)
    {
        close();
        m_socket_fd = other.m_socket_fd;
        m_local_addr = other.m_local_addr;
        m_remote_addr = other.m_remote_addr;
        m_is_bound = other.m_is_bound;

        other.m_socket_fd = -1;
        other.m_is_bound = false;
    }
    return *this;
}

std::expected<void, SocketError> UDPSocket::bind(const IPv4Address &address, uint16_t port)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    m_local_addr.sin_family = AF_INET;
    m_local_addr.sin_port = htons(port);
    m_local_addr.sin_addr.s_addr = htonl(address.to_uint32());

    if (::bind(m_socket_fd, (struct sockaddr *)&m_local_addr, sizeof(m_local_addr)) == -1)
    {
        return std::unexpected(SocketError::kBindFailed);
    }

    m_is_bound = true;
    return {};
}

std::expected<void, SocketError> UDPSocket::connect(const IPv4Address &address, uint16_t port)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    m_remote_addr.sin_family = AF_INET;
    m_remote_addr.sin_port = htons(port);
    m_remote_addr.sin_addr.s_addr = htonl(address.to_uint32());

    // For UDP, connect() sets the default destination
    if (::connect(m_socket_fd, (struct sockaddr *)&m_remote_addr, sizeof(m_remote_addr)) == -1)
    {
        return std::unexpected(SocketError::kConnectFailed);
    }

    return {};
}

std::expected<ssize_t, SocketError> UDPSocket::send(const void *data, size_t size)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    ssize_t result = ::send(m_socket_fd, data, size, 0);
    if (result == -1)
    {
        return std::unexpected(SocketError::kSendFailed);
    }
    return result;
}

std::expected<ssize_t, SocketError> UDPSocket::send_to(const void *data, size_t size, const IPv4Address &address, uint16_t port)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = htonl(address.to_uint32());

    ssize_t result = sendto(m_socket_fd, data, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (result == -1)
    {
        return std::unexpected(SocketError::kSendFailed);
    }
    return result;
}

std::expected<ssize_t, SocketError> UDPSocket::receive(void *buffer, size_t size)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    ssize_t result = recv(m_socket_fd, buffer, size, 0);
    if (result == -1)
    {
        return std::unexpected(SocketError::kReceiveFailed);
    }
    return result;
}

std::expected<UDPSocket::ReceiveFromResult, SocketError> UDPSocket::receive_from(void *buffer, size_t size)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);

    ssize_t bytes_received = recvfrom(m_socket_fd, buffer, size, 0,
                                      (struct sockaddr *)&sender_addr, &sender_len);

    if (bytes_received == -1)
    {
        return std::unexpected(SocketError::kReceiveFailed);
    }

    ReceiveFromResult result;
    result.bytes_received = bytes_received;
    result.sender_address = IPv4Address(ntohl(sender_addr.sin_addr.s_addr));
    result.sender_port = ntohs(sender_addr.sin_port);

    return result;
}

void UDPSocket::close()
{
    if (m_socket_fd != -1)
    {
        ::close(m_socket_fd);
        m_socket_fd = -1;
        m_is_bound = false;
    }
}

bool UDPSocket::is_valid() const
{
    return m_socket_fd != -1;
}

int UDPSocket::get_fd() const
{
    return m_socket_fd;
}

std::expected<IPv4Address, SocketError> UDPSocket::get_local_address() const
{
    if (!m_is_bound)
    {
        return std::unexpected(SocketError::kNotBound);
    }

    return IPv4Address(ntohl(m_local_addr.sin_addr.s_addr));
}

std::expected<uint16_t, SocketError> UDPSocket::get_local_port() const
{
    if (!m_is_bound)
    {
        return std::unexpected(SocketError::kNotBound);
    }

    return ntohs(m_local_addr.sin_port);
}

std::expected<IPv4Address, SocketError> UDPSocket::get_remote_address() const
{
    return IPv4Address(ntohl(m_remote_addr.sin_addr.s_addr));
}

std::expected<uint16_t, SocketError> UDPSocket::get_remote_port() const
{
    return ntohs(m_remote_addr.sin_port);
}

std::expected<void, SocketError> UDPSocket::set_broadcast(bool enable)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    int broadcast = enable ? 1 : 0;
    if (setsockopt(m_socket_fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) != 0)
    {
        return std::unexpected(SocketError::kSocketOptionFailed);
    }
    return {};
}

std::expected<void, SocketError> UDPSocket::set_reuse_address(bool enable)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    int reuse = enable ? 1 : 0;
    if (setsockopt(m_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0)
    {
        return std::unexpected(SocketError::kSocketOptionFailed);
    }
    return {};
}

std::expected<void, SocketError> UDPSocket::set_timeout(int seconds, int microseconds)
{
    if (!is_valid())
    {
        return std::unexpected(SocketError::kInvalidSocket);
    }

    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;

    if (setsockopt(m_socket_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0 ||
        setsockopt(m_socket_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) != 0)
    {
        return std::unexpected(SocketError::kSocketOptionFailed);
    }
    return {};
}