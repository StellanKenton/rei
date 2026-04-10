/************************************************************************************
* @file     : serial.hpp
* @brief    : Serial module public API built on top of the ring buffer.
* @details  : Owns one TX ring buffer and one RX ring buffer and exposes process hooks.
* @author   : GitHub Copilot
* @date     : 2026-04-10
* @version  : V1.0.0
***********************************************************************************/
#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <array>
#include <cstdint>
#include <limits>

#include "rei/tools/ringbuffer/ringbuffer.hpp"

namespace venthmi {

enum class SerialStatus {
    kOk = 0,
    kErrorParam,
    kErrorState
};

struct SerialPortOps {
    using WriteByteFn = bool (*)(void* context, std::uint8_t data);
    using ReadByteFn = bool (*)(void* context, std::uint8_t& data);

    WriteByteFn writeByte{nullptr};
    ReadByteFn readByte{nullptr};
    void* context{nullptr};
};

struct SerialProcessResult {
    std::uint32_t transmittedBytes{0U};
    std::uint32_t receivedBytes{0U};
    std::uint32_t droppedRxBytes{0U};
    bool txBlocked{false};
    bool rxOverflow{false};
    bool stateError{false};
};

class SerialPort {
public:
    using Byte = std::uint8_t;
    static constexpr std::uint32_t kUnlimited = std::numeric_limits<std::uint32_t>::max();
    static constexpr std::uint32_t kDefaultBufferCapacity = 4096U;

    SerialPort() = default;
    explicit SerialPort(SerialPortOps ops) noexcept;
    SerialPort(
        Byte* txStorage,
        std::uint32_t txCapacity,
        Byte* rxStorage,
        std::uint32_t rxCapacity,
        SerialPortOps ops) noexcept;

    SerialPort(const SerialPort&) = delete;
    SerialPort& operator=(const SerialPort&) = delete;

    SerialStatus init(SerialPortOps ops) noexcept;

    SerialStatus init(
        Byte* txStorage,
        std::uint32_t txCapacity,
        Byte* rxStorage,
        std::uint32_t rxCapacity,
        SerialPortOps ops) noexcept;

    template <std::size_t TxSize, std::size_t RxSize>
    SerialStatus init(
        std::array<Byte, TxSize>& txStorage,
        std::array<Byte, RxSize>& rxStorage,
        SerialPortOps ops) noexcept {
        return init(
            txStorage.data(),
            static_cast<std::uint32_t>(TxSize),
            rxStorage.data(),
            static_cast<std::uint32_t>(RxSize),
            ops);
    }

    SerialStatus reset() noexcept;

    [[nodiscard]] bool isConfigured() const noexcept;
    [[nodiscard]] std::uint32_t send(const Byte* data, std::uint32_t length) noexcept;
    [[nodiscard]] std::uint32_t receive(Byte* data, std::uint32_t length) noexcept;
    [[nodiscard]] SerialProcessResult process(
        std::uint32_t maxTxBytes = kUnlimited,
        std::uint32_t maxRxBytes = kUnlimited) noexcept;
    [[nodiscard]] RingBuffer& getReceiveRingBuffer() noexcept;
    [[nodiscard]] const RingBuffer& getReceiveRingBuffer() const noexcept;
    [[nodiscard]] const RingBuffer& getTransmitRingBuffer() const noexcept;

    template <std::size_t N>
    [[nodiscard]] std::uint32_t send(const std::array<Byte, N>& data) noexcept {
        return send(data.data(), static_cast<std::uint32_t>(N));
    }

    template <std::size_t N>
    [[nodiscard]] std::uint32_t receive(std::array<Byte, N>& data) noexcept {
        return receive(data.data(), static_cast<std::uint32_t>(N));
    }

private:
    struct BufferConfig {
        Byte* storage{nullptr};
        std::uint32_t capacity{0U};
    };

    [[nodiscard]] BufferConfig selectBuffer(
        Byte* requestedStorage,
        std::uint32_t requestedCapacity,
        std::array<Byte, kDefaultBufferCapacity>& defaultStorage) noexcept;
    [[nodiscard]] bool hasHandlers() const noexcept;

    std::array<Byte, kDefaultBufferCapacity> defaultTxStorage_{};
    std::array<Byte, kDefaultBufferCapacity> defaultRxStorage_{};
    RingBuffer txRingBuffer_{};
    RingBuffer rxRingBuffer_{};
    SerialPortOps ops_{};
};

const char* toString(SerialStatus status) noexcept;

}  // namespace venthmi

#endif  // SERIAL_HPP
/**************************End of file********************************/