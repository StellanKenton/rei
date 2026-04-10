/************************************************************************************
* @file     : ringbuffer.hpp
* @brief    : Byte-oriented SPSC ring buffer public API.
* @details  : This module provides a reusable ring buffer core for MCU projects.
* @author   : GitHub Copilot
* @date     : 2026-04-10
* @version  : V2.0.0
***********************************************************************************/
#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <atomic>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace venthmi {

enum class RingBufferStatus {
    kOk = 0,
    kErrorParam,
    kErrorEmpty,
    kErrorFull,
    kErrorNoSpace,
    kErrorState
};

class RingBuffer {
public:
    using Byte = std::uint8_t;
    static constexpr std::uint32_t kMaxCapacity = std::numeric_limits<std::uint32_t>::max() / 2U;

    RingBuffer() = default;
    RingBuffer(Byte* storage, std::uint32_t capacity) noexcept;

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    RingBufferStatus init(Byte* storage, std::uint32_t capacity) noexcept;

    template <std::size_t N>
    RingBufferStatus init(std::array<Byte, N>& storage) noexcept {
        return init(storage.data(), static_cast<std::uint32_t>(N));
    }

    RingBufferStatus reset() noexcept;

    std::uint32_t getUsed() const noexcept;
    std::uint32_t getFree() const noexcept;
    std::uint32_t getCapacity() const noexcept;
    bool isEmpty() const noexcept;
    bool isFull() const noexcept;

    RingBufferStatus pushByte(Byte data) noexcept;
    RingBufferStatus popByte(Byte& data) noexcept;
    RingBufferStatus peekByte(Byte& data) const noexcept;

    std::uint32_t write(const Byte* src, std::uint32_t length) noexcept;
    std::uint32_t read(Byte* dst, std::uint32_t length) noexcept;
    std::uint32_t peek(Byte* dst, std::uint32_t length) const noexcept;
    std::uint32_t discard(std::uint32_t length) noexcept;

    // This API changes both indices and therefore requires exclusive access.
    std::uint32_t writeOverwrite(const Byte* src, std::uint32_t length) noexcept;

    template <std::size_t N>
    std::uint32_t write(const std::array<Byte, N>& src) noexcept {
        return write(src.data(), static_cast<std::uint32_t>(N));
    }

    template <std::size_t N>
    std::uint32_t read(std::array<Byte, N>& dst) noexcept {
        return read(dst.data(), static_cast<std::uint32_t>(N));
    }

    template <std::size_t N>
    std::uint32_t peek(std::array<Byte, N>& dst) const noexcept {
        return peek(dst.data(), static_cast<std::uint32_t>(N));
    }

    template <std::size_t N>
    std::uint32_t writeOverwrite(const std::array<Byte, N>& src) noexcept {
        return writeOverwrite(src.data(), static_cast<std::uint32_t>(N));
    }

private:
    [[nodiscard]] bool isConfigured() const noexcept;
    [[nodiscard]] bool hasValidState(std::uint32_t head, std::uint32_t tail) const noexcept;
    [[nodiscard]] bool hasValidState() const noexcept;
    [[nodiscard]] static bool isPowerOfTwo(std::uint32_t capacity) noexcept;
    [[nodiscard]] std::uint32_t getUsedInternal(std::uint32_t head, std::uint32_t tail) const noexcept;
    [[nodiscard]] std::uint32_t toPhysicalIndex(std::uint32_t logicalIndex) const noexcept;
    void copyIn(std::uint32_t logicalHead, const Byte* src, std::uint32_t length) noexcept;
    void copyOut(std::uint32_t logicalTail, Byte* dst, std::uint32_t length) const noexcept;

    Byte* buffer_{nullptr};
    std::uint32_t capacity_{0U};
    std::atomic<std::uint32_t> head_{0U};
    std::atomic<std::uint32_t> tail_{0U};
    std::uint32_t mask_{0U};
    bool isPowerOfTwo_{false};
};

const char* toString(RingBufferStatus status) noexcept;

}  // namespace venthmi

#endif  // RINGBUFFER_HPP
/**************************End of file********************************//************************************************************************************
* @file     : ringbuffer.hpp
* @brief    : Byte-oriented SPSC ring buffer public API.
* @details  : This module provides a reusable ring buffer core for MCU projects.
* @author   : GitHub Copilot
* @date     : 2026-04-10
* @version  : V2.0.0
***********************************************************************************/
#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include <cstdint>

namespace venthmi::tools {

enum class RingBufferStatus : std::uint8_t {
    Ok = 0,
    ErrorParam,
    ErrorEmpty,
    ErrorFull,
    ErrorNoSpace,
    ErrorState
};

class RingBuffer {
public:
    static constexpr std::uint32_t kMaxCapacity = 0x7fffffffU;

    RingBuffer() noexcept = default;
    RingBuffer(std::uint8_t* storage, std::uint32_t capacity) noexcept;

    RingBufferStatus init(std::uint8_t* storage, std::uint32_t capacity) noexcept;
    RingBufferStatus reset() noexcept;

    [[nodiscard]] bool isConfigured() const noexcept;
    [[nodiscard]] std::uint32_t getUsed() const noexcept;
    [[nodiscard]] std::uint32_t getFree() const noexcept;
    [[nodiscard]] std::uint32_t getCapacity() const noexcept;
    [[nodiscard]] bool isEmpty() const noexcept;
    [[nodiscard]] bool isFull() const noexcept;

    RingBufferStatus pushByte(std::uint8_t data) noexcept;
    RingBufferStatus popByte(std::uint8_t& data) noexcept;
    RingBufferStatus peekByte(std::uint8_t& data) const noexcept;

    std::uint32_t write(const std::uint8_t* src, std::uint32_t length) noexcept;
    std::uint32_t read(std::uint8_t* dst, std::uint32_t length) noexcept;
    std::uint32_t peek(std::uint8_t* dst, std::uint32_t length) const noexcept;
    std::uint32_t discard(std::uint32_t length) noexcept;
    std::uint32_t writeOverwrite(const std::uint8_t* src, std::uint32_t length) noexcept;

private:
    [[nodiscard]] bool hasValidState() const noexcept;
    [[nodiscard]] std::uint32_t getUsedInternal() const noexcept;
    [[nodiscard]] std::uint32_t toPhysicalIndex(std::uint32_t logicalIndex) const noexcept;
    [[nodiscard]] std::uint32_t getLinearWriteSpan(std::uint32_t logicalHead) const noexcept;
    [[nodiscard]] std::uint32_t getLinearReadSpan(std::uint32_t logicalTail) const noexcept;

    void copyIn(std::uint32_t logicalHead, const std::uint8_t* src, std::uint32_t length) noexcept;
    void copyOut(std::uint32_t logicalTail, std::uint8_t* dst, std::uint32_t length) const noexcept;

    std::uint8_t* buffer_{nullptr};
    std::uint32_t capacity_{0U};
    volatile std::uint32_t head_{0U};
    volatile std::uint32_t tail_{0U};
    std::uint32_t mask_{0U};
    bool isPowerOfTwo_{false};
};

}  // namespace venthmi::tools

#endif  // RINGBUFFER_HPP
/**************************End of file********************************/