#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

using namespace std::string_literals;

struct ReserveProxyObj
{
public:
    size_t capacity;
    ReserveProxyObj(size_t capacite_to_reserve)
    {
        capacity = capacite_to_reserve;
    }
};

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

public:
    SimpleVector() noexcept = default;

public:
    explicit SimpleVector(size_t size) : size_(size), capacity_(size), array_(size) {
        std::fill(begin(), end(), Type());
    }

    //Copy constructor
    SimpleVector(const SimpleVector& other) : size_(other.GetSize()), capacity_(other.GetSize()), array_(other.GetSize()) {
        std::fill(array_.Get(), array_.Get() + size_, Type());
        std::copy(other.begin(), other.end(), begin());
    }

    SimpleVector(SimpleVector&& other) noexcept : array_(std::move(other.array_)) {
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    //equal operator
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            auto temp(rhs);
            swap(temp);
        }
        return *this;
    }

    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            auto temp(rhs);
            swap(temp);
        }
        return *this;
    }

    // Create size elements vector, initialized by given value
    SimpleVector(size_t size, Type value) : size_(size), capacity_(size), array_(size) {
        std::fill(begin(), end(), std::move(value));
    }

    // Create vector from std::initializer_list
    SimpleVector(std::initializer_list<Type> init) : size_(init.size()), capacity_(init.size()), array_(init.size()) {
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(ReserveProxyObj size_to_res) {
        Reserve(size_to_res.capacity);
    }

public:

    //Insert value on the end
    void PushBack(Type& item) noexcept {
        size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> tmp_arr(capacity_);
            size_t i = 0;
            for (auto it = begin(); it != end(); ++it) {
                *(tmp_arr.Get() + i) = *it;
                ++i;
            }
            array_ = (tmp_arr);
            array_[size_] = item;
        }
        else {
            array_[size_] = item;
        }
        ++size_;
    }

    void PushBack(Type&& item) noexcept {
        size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> tmp_arr(capacity_);
            size_t i = 0;
            for (auto it = begin(); it != end(); ++it) {
                *(tmp_arr.Get() + i) = std::move(*it);
                ++i;
            }
            array_ = std::move(tmp_arr);
            array_[size_] = std::move(item);
        }
        else {
            array_[size_] = std::move(item);
        }
        ++size_;
    }

    Iterator Insert(ConstIterator pos, Type& value) {
        assert(pos <= cend() && pos >= cbegin());
        const size_t new_item_offset = pos - cbegin();
        Iterator mutable_pos = begin() + new_item_offset;
        size_t new_size = size_ + 1;
        if (new_size <= capacity_) {
            std::copy_backward(mutable_pos, begin() + size_, begin() + size_ + 1);
            *(mutable_pos) = value;
        } else {
            capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> tmp_arr(capacity_);
            std::copy(begin(), mutable_pos, tmp_arr.Get());
            *(tmp_arr.Get() + new_item_offset) = value;
            std::copy(mutable_pos, end(), tmp_arr.Get() + new_item_offset + 1);
            array_ = std::copy(tmp_arr);
        }
        ++size_;
        return mutable_pos;
    }

     //Insert value on given pos
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos <= cend() && pos >= cbegin());
        const size_t dist = pos - begin();
        if (capacity_ > size_) {
            std::move_backward(std::make_move_iterator(begin() + dist), std::make_move_iterator(begin() + size_), begin() + size_ + 1);
            *(begin() + dist) = std::move(value);
        }
        else {
            capacity_ = (capacity_ == 0) ? 1 : capacity_ * 2;
            ArrayPtr<Type> tmp_arr(capacity_);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(begin() + dist), tmp_arr.Get());
            *(tmp_arr.Get() + dist) = std::move(value);
            std::move(std::make_move_iterator(begin() + dist), std::make_move_iterator(end()), tmp_arr.Get() + dist + 1);
            array_ = std::move(tmp_arr);
        }
        ++size_;
        return array_.Get() + dist;
    }

    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
        //Erase(end() - 1);
    }

    //Remove vector element in pos
    Iterator Erase(ConstIterator pos) {
        assert(!IsEmpty());
        std::move(Iterator(pos) + 1, end(), Iterator(pos));
        --size_;
        return Iterator(pos);
    }

    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        array_.swap(other.array_);
    }

    // return size_
    size_t GetSize() const noexcept {
        return size_;
    }

    // return capacity_
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return (size_ == 0) ? true : false;
    }

    //Return reference on element with index 'index'
    //Тут не получается кинуть исключение
    Type& operator[](size_t index) noexcept {
        return array_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return array_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Invalid index"s);
        }
        return array_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Invalid index"s);
        }
        return array_[index];
    }

    // Change vector size_ to 0 , not change capacity
    void Clear() noexcept {
        size_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type>tmp_arr(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), tmp_arr.Get());
            capacity_ = new_capacity;
            array_.swap(tmp_arr);
        }
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            capacity_ = std::max(new_size, 2 * capacity_);
            auto tmp_arr = ReallocateCopy(capacity_);
            std::fill(tmp_arr.Get() + size_, tmp_arr.Get() + new_size, Type());
            array_.swap(tmp_arr);
        }
        else if (new_size > size_) {
            std::fill(array_.Get() + size_, array_.Get() + new_size, Type());
        }
        size_ = new_size;
    }

    Iterator begin() noexcept {
        return array_.Get();
    }

    Iterator end() noexcept {
        return begin() + size_;
    }

    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    ConstIterator end() const noexcept {
        return begin() + size_;
    }

    ConstIterator cbegin() const noexcept {
        return array_.Get();
    }

    ConstIterator cend() const noexcept {
        return begin() + size_;
    }

private:
    ArrayPtr<Type> ReallocateCopy(size_t new_capacity) {
        ArrayPtr<Type> result(new_capacity);
        std::move(begin(), end(), result.Get());
        return result;
    }

private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> array_;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.begin() + lhs.GetSize(), rhs.begin(), rhs.begin() + rhs.GetSize());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.begin() + lhs.GetSize(),
        rhs.begin(), rhs.begin() + rhs.GetSize());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs >= lhs;
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {

    return !(lhs < rhs);
}