#pragma once

#include <cstdlib>
#include <utility>

template <typename Type>
class ArrayPtr {
public:
    ArrayPtr() = default;

public:
    //Create in heap array of size elements type "Type"
    //IF size == 0 => raw_ptr_ = nullptr
    explicit ArrayPtr(size_t size) {
        Type* array = new Type[size];
        raw_ptr_ = (size != 0) ? array : nullptr;
    }

    explicit ArrayPtr(Type* raw_ptr) noexcept {
        raw_ptr_ = raw_ptr;
    }

    ArrayPtr(const ArrayPtr&) = delete;
    ArrayPtr(ArrayPtr&& other) noexcept
    {
        raw_ptr_ = std::exchange(other.raw_ptr_, {});
    }

    ~ArrayPtr() {
        delete[] raw_ptr_;
    }

    ArrayPtr& operator=(const ArrayPtr&) = delete;
    ArrayPtr& operator=(ArrayPtr&& rhs) noexcept
    {
        raw_ptr_ = std::exchange(rhs.raw_ptr_, {});
        return *this;
    }

public:

    [[nodiscard]] Type* Release() noexcept {
        auto result_ptr = raw_ptr_;
        raw_ptr_ = nullptr;
        return result_ptr;
    }

    Type& operator[](size_t index) noexcept {
        return *(raw_ptr_ + index);
    }

    const Type& operator[](size_t index) const noexcept {
        return *(raw_ptr_ + index);
    }

    explicit operator bool() const {
        return (raw_ptr_ == nullptr);
    }

    Type* Get() const noexcept {
        return raw_ptr_;
    }

    // Swap raw_ptr_ with other ArrayPtr
    void swap(ArrayPtr& other) noexcept {
        std::swap(this->raw_ptr_, other.raw_ptr_);
    }

private:
    Type* raw_ptr_ = nullptr;
};
