#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <utility>
#include <iterator>

#include "array_ptr.h"

class ReserveProxyObj {

public:
    ReserveProxyObj(const size_t capacity_to_reserve)
        :capacity_to_reserve_(capacity_to_reserve) {}

    size_t GetCapacity() {
        return capacity_to_reserve_;
    }

private:
    size_t capacity_to_reserve_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}


template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) {
        ArrayPtr<Type> array_ptr(size);
        FillWithDefaultValue(&array_ptr[0], &array_ptr[size]);
        array_ptr_.swap(array_ptr);
        size_ = size;
        capacity_ = size;
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) : size_(size), capacity_(size) {
        ArrayPtr<Type> array_ptr(size);
        std::fill(&array_ptr[0], &array_ptr[size], value);
        array_ptr_.swap(array_ptr);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) {
        SimpleVector<Type> new_vector(init.size());
        std::copy(init.begin(), init.end(), new_vector.begin());
        swap(new_vector);
    }

    SimpleVector(ReserveProxyObj object) {
        Reserve(object.GetCapacity());
    }

    // Копирующий конструктор
    SimpleVector(const SimpleVector& other) 
        : array_ptr_(other.size_)
        , size_(other.size_)
        , capacity_(other.size_)
    {
        std::copy(other.begin(), other.end(), array_ptr_.Get());
    }

    // Конструктор перемещения
    SimpleVector(SimpleVector&& other) noexcept {
        array_ptr_ = std::move(other.array_ptr_);
        size_ = std::exchange(other.size_, 0);
        capacity_ = std::exchange(other.capacity_, 0);
    }

    // Оператор присваивания
    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            if (rhs.IsEmpty()) {
                Clear();
                size_ = 0;
                capacity_ = 0;
                return *this;
            }
            auto rhs_copy(rhs);
            swap(rhs_copy);
        }
        return *this;
    }

    // Перемещающий оператор присваивания
    SimpleVector& operator=(SimpleVector&& rhs) noexcept {
        if (this != &rhs) {
            array_ptr_ = std::move(rhs.array_ptr_);
            size_ = std::exchange(rhs.size_, 0);
            capacity_ = std::exchange(rhs.capacity_, 0);
        }
        return *this;
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return array_ptr_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index < size_) {
            return array_ptr_[index];
        }
        else {
            throw std::out_of_range("Out of range array");
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index < size_) {
            return array_ptr_[index];
        }
        else {
            throw std::out_of_range("Out of range array");
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size > size_) {
            if (new_size > capacity_) {
                size_t max_size = std::max(2 * capacity_, new_size);
                ArrayPtr<Type> array_bigger(max_size);
                std::move(begin(), end(), array_bigger.Get());
                FillWithDefaultValue(&array_bigger[size_], &array_bigger[max_size]);
                array_ptr_.swap(array_bigger);
                size_ = new_size;
                capacity_ = max_size;
            }
            else {
                FillWithDefaultValue(end(), &array_ptr_[new_size]);
                size_ = new_size;
            }
        }
        else {
            size_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return array_ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return array_ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return array_ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return array_ptr_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return array_ptr_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return array_ptr_.Get() + size_;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (capacity_ == 0) {
            SimpleVector<Type> new_vector(1);
            new_vector[0] = item;
            swap(new_vector);
        }
        else if (capacity_ == size_) {
            SimpleVector<Type> new_vector(2 * capacity_);
            std::copy(begin(), end(), new_vector.begin());
            new_vector[size_] = item;
            new_vector.size_ = size_ + 1;
            swap(new_vector);
        }
        else {
            array_ptr_[size_] = item;
            ++size_;
        }
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type&& item) {
        if (capacity_ == 0) {
            SimpleVector<Type> new_vector(1);
            new_vector[0] = std::move(item);
            *this = std::move(new_vector);
        }
        else if (size_ == capacity_) {
            SimpleVector<Type> new_vector(2 * capacity_);
            std::move(std::make_move_iterator(begin()), std::make_move_iterator(end()), new_vector.begin());
            new_vector[size_] = std::move(item);
            new_vector.size_ = size_ + 1;
            *this = std::move(new_vector);
        }
        else {
            array_ptr_[size_] = std::move(item);
            ++size_;
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t pos_distance = std::distance(cbegin(), pos);
        if (capacity_ == 0) {
            SimpleVector<Type> new_vector(1);
            new_vector[0] = value;
            swap(new_vector);
        }
        else if (capacity_ == size_) {
            SimpleVector<Type> new_vector(2 * capacity_);
            std::copy(begin(), begin() + pos_distance, new_vector.begin());
            new_vector[pos_distance] = value;
            std::copy_backward(begin() + pos_distance, end(), new_vector.begin() + size_ + 1);
            new_vector.size_ = size_ + 1;
            swap(new_vector);
        }
        else {
            std::copy_backward(begin() + pos_distance, end(), array_ptr_.Get() + pos_distance + 1);
            array_ptr_[pos_distance] = value;
            ++size_;
        }
        return &array_ptr_[pos_distance];
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t pos_distance = std::distance(cbegin(), pos);
        if (capacity_ == 0) {
            SimpleVector<Type> new_vector(1);
            new_vector[0] = std::move(value);
            *this = std::move(new_vector);
        }
        else if (capacity_ == size_) {
            SimpleVector<Type> new_vector(2 * capacity_);
            std::move(begin(), begin() + pos_distance, new_vector.begin());
            new_vector[pos_distance] = std::move(value);
            std::move_backward(begin() + pos_distance, end(), new_vector.begin() + size_ + 1);
            new_vector.size_ = size_ + 1;
            *this = std::move(new_vector);
        }
        else {
            std::move_backward(begin() + pos_distance, end(), array_ptr_.Get() + size_ + 1);
            array_ptr_[pos_distance] = std::move(value);
            ++size_;
        }
        return &array_ptr_[pos_distance];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t pos_distance = std::distance(cbegin(), pos);
        std::move(begin() + pos_distance + 1, end(), &array_ptr_[pos_distance]);
        --size_;

        return &array_ptr_[pos_distance];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        array_ptr_.swap(other.array_ptr_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    // Задает ёмкость вектора
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector<Type> new_vector(new_capacity);
            std::move(begin(), end(), new_vector.begin());
            new_vector.size_ = size_;
            swap(new_vector);
        }
    }

private:
    ArrayPtr<Type> array_ptr_;
    size_t size_ = 0;
    size_t capacity_ = 0;

    static void FillWithDefaultValue(Type* from, Type* to)
    {
        while (from != to) {
            *from = Type{};
            ++from;
        }
    }
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
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
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}
