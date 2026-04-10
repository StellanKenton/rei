/***********************************************************************************
* @file     : serial.cpp
* @brief    : Serial module implementation.
* @details  : Moves TX data from a public send API into a TX ring buffer and polls RX.
* @author   : GitHub Copilot
* @date     : 2026-04-10
* @version  : V1.0.0
**********************************************************************************/
#include "serial.hpp"

#include <algorithm>

namespace venthmi {

SerialPort::SerialPort(SerialPortOps ops) noexcept {
    (void)init(ops);
}

SerialPort::SerialPort(
    Byte* txStorage,
    std::uint32_t txCapacity,
    Byte* rxStorage,
    std::uint32_t rxCapacity,
    SerialPortOps ops) noexcept {
    (void)init(txStorage, txCapacity, rxStorage, rxCapacity, ops);
}

SerialStatus SerialPort::init(SerialPortOps ops) noexcept {
    return init(nullptr, 0U, nullptr, 0U, ops);
}

SerialStatus SerialPort::init(
    Byte* txStorage,
    std::uint32_t txCapacity,
    Byte* rxStorage,
    std::uint32_t rxCapacity,
    SerialPortOps ops) noexcept {
    if ((ops.writeByte == nullptr) || (ops.readByte == nullptr)) {
        ops_ = {};
        (void)txRingBuffer_.init(nullptr, 0U);
        (void)rxRingBuffer_.init(nullptr, 0U);
        return SerialStatus::kErrorParam;
    }

    const auto txBuffer = selectBuffer(txStorage, txCapacity, defaultTxStorage_);
    const auto rxBuffer = selectBuffer(rxStorage, rxCapacity, defaultRxStorage_);
    if ((txBuffer.storage == nullptr) || (rxBuffer.storage == nullptr)) {
        ops_ = {};
        (void)txRingBuffer_.init(nullptr, 0U);
        (void)rxRingBuffer_.init(nullptr, 0U);
        return SerialStatus::kErrorParam;
    }

    if (txRingBuffer_.init(txBuffer.storage, txBuffer.capacity) != RingBufferStatus::kOk) {
        ops_ = {};
        (void)rxRingBuffer_.init(nullptr, 0U);
        return SerialStatus::kErrorParam;
    }

    if (rxRingBuffer_.init(rxBuffer.storage, rxBuffer.capacity) != RingBufferStatus::kOk) {
        ops_ = {};
        (void)txRingBuffer_.init(nullptr, 0U);
        return SerialStatus::kErrorParam;
    }

    ops_ = ops;
    return SerialStatus::kOk;
}

SerialStatus SerialPort::reset() noexcept {
    if (!isConfigured()) {
        return SerialStatus::kErrorParam;
    }

    if ((txRingBuffer_.reset() != RingBufferStatus::kOk) || (rxRingBuffer_.reset() != RingBufferStatus::kOk)) {
        return SerialStatus::kErrorState;
    }

    return SerialStatus::kOk;
}

bool SerialPort::isConfigured() const noexcept {
    return hasHandlers() && txRingBuffer_.getCapacity() != 0U && rxRingBuffer_.getCapacity() != 0U;
}

std::uint32_t SerialPort::send(const Byte* data, std::uint32_t length) noexcept {
    if (!isConfigured()) {
        return 0U;
    }

    return txRingBuffer_.write(data, length);
}

std::uint32_t SerialPort::receive(Byte* data, std::uint32_t length) noexcept {
    if (!isConfigured()) {
        return 0U;
    }

    return rxRingBuffer_.read(data, length);
}

SerialProcessResult SerialPort::process(std::uint32_t maxTxBytes, std::uint32_t maxRxBytes) noexcept {
    SerialProcessResult result{};
    if (!isConfigured()) {
        result.stateError = true;
        return result;
    }

    const auto txLimit = std::min(maxTxBytes, txRingBuffer_.getUsed());
    for (std::uint32_t index = 0U; index < txLimit; ++index) {
        Byte txByte{0U};
        if (txRingBuffer_.peekByte(txByte) != RingBufferStatus::kOk) {
            result.stateError = true;
            break;
        }

        if (!ops_.writeByte(ops_.context, txByte)) {
            result.txBlocked = true;
            break;
        }

        if (txRingBuffer_.discard(1U) != 1U) {
            result.stateError = true;
            break;
        }

        ++result.transmittedBytes;
    }

    for (std::uint32_t index = 0U; index < maxRxBytes; ++index) {
        Byte rxByte{0U};
        if (!ops_.readByte(ops_.context, rxByte)) {
            break;
        }

        if (rxRingBuffer_.pushByte(rxByte) != RingBufferStatus::kOk) {
            result.rxOverflow = true;
            ++result.droppedRxBytes;
            continue;
        }

        ++result.receivedBytes;
    }

    return result;
}

RingBuffer& SerialPort::getReceiveRingBuffer() noexcept {
    return rxRingBuffer_;
}

const RingBuffer& SerialPort::getReceiveRingBuffer() const noexcept {
    return rxRingBuffer_;
}

const RingBuffer& SerialPort::getTransmitRingBuffer() const noexcept {
    return txRingBuffer_;
}

SerialPort::BufferConfig SerialPort::selectBuffer(
    Byte* requestedStorage,
    std::uint32_t requestedCapacity,
    std::array<Byte, kDefaultBufferCapacity>& defaultStorage) noexcept {
    if ((requestedStorage == nullptr) && (requestedCapacity == 0U)) {
        return {defaultStorage.data(), kDefaultBufferCapacity};
    }

    if ((requestedStorage == nullptr) || (requestedCapacity == 0U)) {
        return {};
    }

    return {requestedStorage, requestedCapacity};
}

bool SerialPort::hasHandlers() const noexcept {
    return (ops_.writeByte != nullptr) && (ops_.readByte != nullptr);
}

const char* toString(SerialStatus status) noexcept {
    switch (status) {
        case SerialStatus::kOk:
            return "ok";
        case SerialStatus::kErrorParam:
            return "error_param";
        case SerialStatus::kErrorState:
            return "error_state";
    }

    return "unknown";
}

}  // namespace venthmi
/**************************End of file********************************/