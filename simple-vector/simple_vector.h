#pragma once

#include <stdexcept>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <utility>

#include "array_ptr.h"

using namespace std;

class ReserveProxyObj {
public:
    ReserveProxyObj(const size_t capacity_to_reserve) 
    :reverse_assistant_(capacity_to_reserve) {}
 
    size_t GetSize() { 
        return reverse_assistant_; 
    }
    
private:
    size_t reverse_assistant_;
    
};
 
ReserveProxyObj Reserve(const size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;
 
    SimpleVector(ReserveProxyObj obj) {
        SimpleVector<Type> vec;
        vec.Reserve(obj.GetSize());
        swap(vec);
    }
    
    explicit SimpleVector(size_t size) : SimpleVector(size, Type{}) {}

    SimpleVector(size_t size, const Type& value) : items_(size) , size_(size) , capacity_(size)
    {
        fill(items_.Get(), items_.Get() + size_, value);
    }

    SimpleVector(std::initializer_list<Type> init) : items_(init.size()) , size_(init.size()) , capacity_(init.size())
    {
        ConstructorAssistant(init);
    }
    
    SimpleVector(const SimpleVector& other): items_(other.size_) , size_(other.size_) , capacity_(other.size_)
    {
        ConstructorAssistant(other);
    }
    
    SimpleVector(SimpleVector&& other){
        items_ = move(other.items_);
		size_ = exchange(other.size_, 0);
		capacity_ = exchange(other.capacity_, 0);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                return *this;
            }
            SimpleVector<Type> arr_ptr(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), arr_ptr.begin());
            arr_ptr.capacity_ = rhs.capacity_;
            swap(arr_ptr);
        }
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                return *this;
            }
            SimpleVector<Type> arr_ptr(rhs.size_);
            std::copy(rhs.begin(), rhs.end(), arr_ptr.begin());
            arr_ptr.capacity_ = rhs.capacity_;
            swap(arr_ptr);
        }
        return *this;
    }
    
    void Reserve(size_t capacity_to_reserve) {
        if (capacity_to_reserve > capacity_) {
            SimpleVector<Type> tmp_items(capacity_to_reserve);
            std::copy(cbegin(), cend(), tmp_items.begin());
            tmp_items.size_ = size_;
            swap(tmp_items);
        }
    }
    
    template <typename Container>
    void ConstructorAssistant(Container& other) {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    Type& operator[](size_t index) noexcept {
        assert(index <= capacity_);
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        assert(index <= capacity_);
        return items_[index];
    }

    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("error"); 
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("error");
        }
        return items_[index];
    }

    void Clear() noexcept {
        size_ = 0;
    }

    void Resize(size_t new_size) {                                                             
        if (new_size == size_) return;
        
        if (new_size < size_) {
            for (auto it = &items_[new_size]; it != &items_[size_]; ++it) {
                *(it) = move(Type{});
            }
        }
        
        if (new_size > capacity_) {
            auto new_capacity = max(new_size, 2 * capacity_);
            ArrayPtr<Type> arr_ptr(new_capacity);
            move(&items_[0], &items_[size_], &arr_ptr[0]);
            for (auto it = &arr_ptr[size_]; it != &arr_ptr[new_size]; ++it) {
                *(it) = move(Type{});
            }
            items_.swap(arr_ptr);
            capacity_ = new_capacity;
        }
        
        size_ = new_size;
    }

    Iterator begin() noexcept {
        return Iterator{items_.Get()};
    }

    Iterator end() noexcept {
        return Iterator{&items_[size_]};
    }

    ConstIterator begin() const noexcept {
        return ConstIterator{items_.Get()};
    }

    ConstIterator end() const noexcept {
        return ConstIterator{&items_[size_]};
    }

    ConstIterator cbegin() const noexcept {
        return ConstIterator{items_.Get()};
    }

    ConstIterator cend() const noexcept {
        return ConstIterator{&items_[size_]};
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {
            items_[size_] = item;
        } else {
            auto new_capacity = max(size_t(1), 2 * capacity_); 
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::copy(&items_[0], &items_[size_], &arr_ptr[0]);
            arr_ptr[size_] = item;
            items_.swap(arr_ptr);
            capacity_ = new_capacity;
        }
        ++size_;
    }
    
    void PushBack(Type&& item) {
        if (size_ < capacity_) {
            items_[size_] = move(item);
        } else {
            auto new_capacity = max(size_t(1), 2 * capacity_); 
            ArrayPtr<Type> arr_ptr(new_capacity);
            move(&items_[0], &items_[size_], &arr_ptr[0]);
            arr_ptr[size_] = move(item);
            items_.swap(arr_ptr);
            capacity_ = new_capacity;
        }
        ++size_;
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= cbegin() && pos <= cend());
        
        auto pos_element = distance(cbegin(), pos);
        
        if (size_ < capacity_) {
            std::copy_backward(pos, cend(), &items_[(size_ + 1)]);
            items_[pos_element] = value;
        } else {
            auto new_capacity = max(size_t(1), 2 * capacity_); 
            ArrayPtr<Type> arr_ptr(new_capacity);
            std::copy(&items_[0], &items_[pos_element], &arr_ptr[0]);
            std::copy_backward(pos, cend(), &arr_ptr[(size_ + 1)]);
            arr_ptr[pos_element] = value;
            items_.swap(arr_ptr);
            capacity_ = new_capacity;
        }
        
        ++size_;
        return Iterator{&items_[pos_element]};
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= cbegin() && pos <= cend());
        
        auto no_const_pos = const_cast<Iterator>(pos);
        auto pos_element = distance(begin(), no_const_pos);
        
        if (size_ < capacity_) {
            move_backward(no_const_pos, end(), &items_[(size_ + 1)]);
            items_[pos_element] = move(value);
        } else {
            auto new_capacity = max(size_t(1), 2 * capacity_); 
            ArrayPtr<Type> arr_ptr(new_capacity);
            move(&items_[0], &items_[pos_element], &arr_ptr[0]);
            move_backward(no_const_pos, end(), &arr_ptr[(size_ + 1)]);
            arr_ptr[pos_element] = move(value);
            items_.swap(arr_ptr);
            capacity_ = new_capacity;
        }
        
        ++size_;
        return Iterator{&items_[pos_element]};
    }

    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;  
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= cbegin() && pos < cend());
        auto no_const_pos = const_cast<Iterator>(pos);
        auto pos_element = distance(begin(), no_const_pos);
        move(++no_const_pos, end(), &items_[pos_element]);
        --size_;
        return &items_[pos_element];
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
    
private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) return false;
    return std::equal(lhs.begin(),lhs.end(),rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs > lhs);
}