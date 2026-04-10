/***********************************************************************************
* @file     : ringbuffer.cpp
* @brief    : Byte-oriented SPSC ring buffer implementation.
* @details  : The core logic is portable and does not depend on an RTOS.
* @author   : GitHub Copilot
* @date     : 2026-04-10
* @version  : V2.0.0
**********************************************************************************/
#include "ringbuffer.hpp"

#include <algorithm>
#include <cstring>

namespace venthmi {

RingBuffer::RingBuffer(Byte* storage, std::uint32_t capacity) noexcept {
    (void)init(storage, capacity);
}

RingBufferStatus RingBuffer::init(Byte* storage, std::uint32_t capacity) noexcept {
    if ((storage == nullptr) || (capacity == 0U) || (capacity > kMaxCapacity)) {
        buffer_ = nullptr;
        capacity_ = 0U;
        head_.store(0U, std::memory_order_relaxed);
        tail_.store(0U, std::memory_order_relaxed);
        mask_ = 0U;
        isPowerOfTwo_ = false;
        return RingBufferStatus::kErrorParam;
    }

    buffer_ = storage;
    capacity_ = capacity;
    head_.store(0U, std::memory_order_relaxed);
    tail_.store(0U, std::memory_order_relaxed);
    isPowerOfTwo_ = isPowerOfTwo(capacity_);
    mask_ = isPowerOfTwo_ ? (capacity_ - 1U) : 0U;

    return RingBufferStatus::kOk;
}

RingBufferStatus RingBuffer::reset() noexcept {
    if (!isConfigured()) {
        return RingBufferStatus::kErrorParam;
    }

    head_.store(0U, std::memory_order_relaxed);
    tail_.store(0U, std::memory_order_relaxed);
    return RingBufferStatus::kOk;
}

std::uint32_t RingBuffer::getUsed() const noexcept {
    const auto head = head_.load(std::memory_order_acquire);
    const auto tail = tail_.load(std::memory_order_acquire);
    if (!hasValidState(head, tail)) {
        return 0U;
    }

    return getUsedInternal(head, tail);
}

std::uint32_t RingBuffer::getFree() const noexcept {
    const auto head = head_.load(std::memory_order_acquire);
    const auto tail = tail_.load(std::memory_order_acquire);
    if (!hasValidState(head, tail)) {
        return 0U;
    }

    return capacity_ - getUsedInternal(head, tail);
}

std::uint32_t RingBuffer::getCapacity() const noexcept {
    return isConfigured() ? capacity_ : 0U;
}

bool RingBuffer::isEmpty() const noexcept {
    const auto head = head_.load(std::memory_order_acquire);
    const auto tail = tail_.load(std::memory_order_acquire);
    if (!hasValidState(head, tail)) {
        return false;
    }

    return getUsedInternal(head, tail) == 0U;
}

bool RingBuffer::isFull() const noexcept {
    const auto head = head_.load(std::memory_order_acquire);
    const auto tail = tail_.load(std::memory_order_acquire);
    if (!hasValidState(head, tail)) {
        return false;
    }

    return getUsedInternal(head, tail) == capacity_;
}

RingBufferStatus RingBuffer::pushByte(Byte data) noexcept {
    if (!isConfigured()) {
        return RingBufferStatus::kErrorParam;
    }

    const auto head = head_.load(std::memory_order_relaxed);
    const auto tail = tail_.load(std::memory_order_acquire);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return RingBufferStatus::kErrorState;
    }
    if (used == capacity_) {
        return RingBufferStatus::kErrorFull;
    }

    buffer_[toPhysicalIndex(head)] = data;
    head_.store(head + 1U, std::memory_order_release);
    return RingBufferStatus::kOk;
}

RingBufferStatus RingBuffer::popByte(Byte& data) noexcept {
    if (!isConfigured()) {
        return RingBufferStatus::kErrorParam;
    }

    const auto tail = tail_.load(std::memory_order_relaxed);
    const auto head = head_.load(std::memory_order_acquire);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return RingBufferStatus::kErrorState;
    }
    if (used == 0U) {
        return RingBufferStatus::kErrorEmpty;
    }

    data = buffer_[toPhysicalIndex(tail)];
    tail_.store(tail + 1U, std::memory_order_release);
    return RingBufferStatus::kOk;
}

RingBufferStatus RingBuffer::peekByte(Byte& data) const noexcept {
    if (!isConfigured()) {
        return RingBufferStatus::kErrorParam;
    }

    const auto tail = tail_.load(std::memory_order_acquire);
    const auto head = head_.load(std::memory_order_acquire);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return RingBufferStatus::kErrorState;
    }
    if (used == 0U) {
        return RingBufferStatus::kErrorEmpty;
    }

    data = buffer_[toPhysicalIndex(tail)];
    return RingBufferStatus::kOk;
}

std::uint32_t RingBuffer::write(const Byte* src, std::uint32_t length) noexcept {
    if (!isConfigured() || ((src == nullptr) && (length != 0U))) {
        return 0U;
    }

    if (length == 0U) {
        return 0U;
    }

    const auto head = head_.load(std::memory_order_relaxed);
    const auto tail = tail_.load(std::memory_order_acquire);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return 0U;
    }

    const auto free = capacity_ - used;
    const auto writeLength = std::min(length, free);
    if (writeLength == 0U) {
        return 0U;
    }

    copyIn(head, src, writeLength);
    head_.store(head + writeLength, std::memory_order_release);
    return writeLength;
}

std::uint32_t RingBuffer::read(Byte* dst, std::uint32_t length) noexcept {
    if (!isConfigured() || ((dst == nullptr) && (length != 0U))) {
        return 0U;
    }

    if (length == 0U) {
        return 0U;
    }

    const auto tail = tail_.load(std::memory_order_relaxed);
    const auto head = head_.load(std::memory_order_acquire);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return 0U;
    }

    const auto readLength = std::min(length, used);
    if (readLength == 0U) {
        return 0U;
    }

    copyOut(tail, dst, readLength);
    tail_.store(tail + readLength, std::memory_order_release);
    return readLength;
}

std::uint32_t RingBuffer::peek(Byte* dst, std::uint32_t length) const noexcept {
    if (!isConfigured() || ((dst == nullptr) && (length != 0U))) {
        return 0U;
    }

    if (length == 0U) {
        return 0U;
    }

    const auto tail = tail_.load(std::memory_order_acquire);
    const auto head = head_.load(std::memory_order_acquire);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return 0U;
    }

    const auto readLength = std::min(length, used);
    if (readLength == 0U) {
        return 0U;
    }

    copyOut(tail, dst, readLength);
    return readLength;
}

std::uint32_t RingBuffer::discard(std::uint32_t length) noexcept {
    if (!isConfigured() || (length == 0U)) {
        return 0U;
    }

    const auto tail = tail_.load(std::memory_order_relaxed);
    const auto head = head_.load(std::memory_order_acquire);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return 0U;
    }

    const auto discardLength = std::min(length, used);
    if (discardLength == 0U) {
        return 0U;
    }

    tail_.store(tail + discardLength, std::memory_order_release);
    return discardLength;
}

std::uint32_t RingBuffer::writeOverwrite(const Byte* src, std::uint32_t length) noexcept {
    if (!isConfigured() || ((src == nullptr) && (length != 0U))) {
        return 0U;
    }

    if (length == 0U) {
        return 0U;
    }

    const auto inputLength = length;

    auto tail = tail_.load(std::memory_order_relaxed);
    const auto head = head_.load(std::memory_order_relaxed);
    const auto used = getUsedInternal(head, tail);
    if (used > capacity_) {
        return 0U;
    }

    auto writeLength = inputLength;
    auto writeSource = src;
    if (writeLength >= capacity_) {
        writeSource = src + (writeLength - capacity_);
        writeLength = capacity_;
    }

    const auto free = capacity_ - used;
    const auto overflowLength = writeLength > free ? (writeLength - free) : 0U;
    if (overflowLength != 0U) {
        tail += overflowLength;
    }

    copyIn(head, writeSource, writeLength);
    const auto newHead = head + writeLength;
    if (newHead - tail > capacity_) {
        tail = newHead - capacity_;
    }

    tail_.store(tail, std::memory_order_relaxed);
    head_.store(newHead, std::memory_order_release);
    return inputLength;
}

bool RingBuffer::isConfigured() const noexcept {
    return (buffer_ != nullptr) && (capacity_ != 0U);
}

bool RingBuffer::hasValidState(std::uint32_t head, std::uint32_t tail) const noexcept {
    if (!isConfigured()) {
        return false;
    }

    return getUsedInternal(head, tail) <= capacity_;
}

bool RingBuffer::hasValidState() const noexcept {
    return hasValidState(head_.load(std::memory_order_acquire), tail_.load(std::memory_order_acquire));
}

bool RingBuffer::isPowerOfTwo(std::uint32_t capacity) noexcept {
    return (capacity != 0U) && ((capacity & (capacity - 1U)) == 0U);
}

std::uint32_t RingBuffer::getUsedInternal(std::uint32_t head, std::uint32_t tail) const noexcept {
    return head - tail;
}

std::uint32_t RingBuffer::toPhysicalIndex(std::uint32_t logicalIndex) const noexcept {
    if (isPowerOfTwo_) {
        return logicalIndex & mask_;
    }

    return logicalIndex % capacity_;
}

void RingBuffer::copyIn(std::uint32_t logicalHead, const Byte* src, std::uint32_t length) noexcept {
    if (length == 0U) {
        return;
    }

    const auto physicalHead = toPhysicalIndex(logicalHead);
    const auto linearSpan = capacity_ - physicalHead;
    const auto firstLength = std::min(length, linearSpan);

    (void)std::memcpy(buffer_ + physicalHead, src, firstLength);
    if (length > firstLength) {
        (void)std::memcpy(buffer_, src + firstLength, length - firstLength);
    }
}

void RingBuffer::copyOut(std::uint32_t logicalTail, Byte* dst, std::uint32_t length) const noexcept {
    if (length == 0U) {
        return;
    }

    const auto physicalTail = toPhysicalIndex(logicalTail);
    const auto linearSpan = capacity_ - physicalTail;
    const auto firstLength = std::min(length, linearSpan);

    (void)std::memcpy(dst, buffer_ + physicalTail, firstLength);
    if (length > firstLength) {
        (void)std::memcpy(dst + firstLength, buffer_, length - firstLength);
    }
}

const char* toString(RingBufferStatus status) noexcept {
    switch (status) {
        case RingBufferStatus::kOk:
            return "ok";
        case RingBufferStatus::kErrorParam:
            return "error_param";
        case RingBufferStatus::kErrorEmpty:
            return "error_empty";
        case RingBufferStatus::kErrorFull:
            return "error_full";
        case RingBufferStatus::kErrorNoSpace:
            return "error_no_space";
        case RingBufferStatus::kErrorState:
            return "error_state";
    }

    return "unknown";
}

}  // namespace venthmi
/**************************End of file********************************/