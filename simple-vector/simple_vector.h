#pragma once
#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <stdexcept>

class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity)
        : capacity_(capacity)
    {
    }
    
    size_t GetCapacity() {
        return capacity_;
    }
    
private:
    size_t capacity_;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size)
        : SimpleVector(size, Type())
    {
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value)
        : items_(size)
        , size_(size)
        , capacity_(size)
    {
        std::fill(items_.Get(), items_.Get() + size, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
        : items_(init.size())
        , size_(init.size())
        , capacity_(init.size())
    {
        std::copy(init.begin(), init.end(), items_.Get());
    }
    
    // конструктор копирования
    SimpleVector(const SimpleVector& other)
        : items_(other.capacity_)
        , size_(other.size_)
    {
        std::copy(other.begin(), other.end(), items_.Get());
    }

    // конструктор перемещения
    SimpleVector(SimpleVector&& other)
        : items_(other.capacity_)
    {
        swap(other);
    }
    
    // Конструктор с вызовом функции Reserve
    explicit SimpleVector(ReserveProxyObj obj)
    {
        Reserve(obj.GetCapacity());
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
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) throw std::out_of_range ("Index more size");
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) throw std::out_of_range ("Index more size");
        return items_[index];
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
       if (new_size <= size_) {
            size_ = new_size;
        }
        else if (new_size < capacity_) {
			std::for_each(items_.Get() + size_, items_.Get() + new_size, [](auto& x) {x = Type{};});
			size_ = new_size;
		}
        else {
			ArrayPtr<Type> temp(new_size);
			std::move(items_.Get(), items_.Get() + size_, temp.Get());
			std::for_each(temp.Get() + size_, temp.Get() + new_size, [](auto& x) {x = Type{};});
			items_.swap(temp);
			size_ = new_size;
			capacity_ = new_size;
		}
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }

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

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (size_ + 1 > capacity_) {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            std::fill(temp.Get(), temp.Get() + new_capacity, Type());
            std::copy(items_.Get(), items_.Get() + size_, temp.Get());
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        items_[size_] = item;
        ++size_;
    }
    
    // метод PushBack путем перемещения
    void PushBack(Type&& item) {
        if (size_ + 1 > capacity_) {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(new_capacity);
            std::move(items_.Get(), items_.Get() + size_, temp.Get());
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        items_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        size_t count = pos - items_.Get();
        if (capacity_ == 0) {
            ArrayPtr<Type> temp(1);
            temp[count] = value;
            items_.swap(temp);
            ++capacity_;
        }
        else if (size_ < capacity_) {
            std::copy_backward(items_.Get() + count, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[count] = value;
        }
        else {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(capacity_);
            std::copy(items_.Get(), items_.Get() + size_, temp.Get());
            std::copy_backward(items_.Get() + count, items_.Get() + size_, temp.Get() + size_ + 1);
            temp[count] = value;
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;
        return &items_[count];
    }
    
    // метод Insert путем перемещения
    Iterator Insert(Iterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t count = pos - items_.Get();
        if (capacity_ == 0) {
            ArrayPtr<Type> temp(1);
            temp[count] = std::move(value);
            items_.swap(temp);
            ++capacity_;
        }
        else if (size_ < capacity_) {
            std::move_backward(items_.Get() + count, items_.Get() + size_, items_.Get() + size_ + 1);
            items_[count] = std::move(value);
        }
        else {
            size_t new_capacity = std::max(size_ + 1, capacity_ * 2);
            ArrayPtr<Type> temp(capacity_);
            std::move(items_.Get(), items_.Get() + size_, temp.Get());
            std::move_backward(items_.Get() + count, items_.Get() + size_, temp.Get() + size_ + 1);
            temp[count] = std::move(value);
            items_.swap(temp);
            capacity_ = new_capacity;
        }
        ++size_;
        return &items_[count];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        assert(!IsEmpty());
        --size_;
    }

     // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        std::move(Iterator(pos) + 1, end(), Iterator(pos));
        --size_;
        return Iterator(pos);
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }
    
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> temp(new_capacity);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(end()), temp.Get());
            items_.swap(temp);
            capacity_ = new_capacity;
        }
    }
    
    private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
     return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs >= lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (rhs < lhs);
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
} 
