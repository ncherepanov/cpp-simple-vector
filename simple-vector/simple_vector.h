#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <utility>

#include "array_ptr.h"

struct ReserveProxyObj{
    explicit ReserveProxyObj(size_t capacity)
        :capacity_(capacity){ }
    size_t capacity_ = 0;    
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
    : SimpleVector(size, Type{})  {  }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
    : size_(size), capacity_(size), array_(size) {
        std::fill_n(array_.Get(), size, value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
    : size_(init.size()), capacity_(init.size()), array_(init.size()){
        std::copy(init.begin(), init.end(), begin());
    }
    
    explicit SimpleVector(ReserveProxyObj object){
        Reserve(object.capacity_);
    }
    
    void Reserve(size_t new_capacity){
        if(new_capacity > capacity_){
            size_t size_tmp = size_;
            this -> Resize(new_capacity);
            size_ = std::move(size_tmp);
        } 
    }
    
    SimpleVector(const SimpleVector& other) 
    :size_(other.size_), capacity_(other.capacity_), array_(other.capacity_) {
        std::copy(other.begin(), other.end(), begin());
    }
    
    SimpleVector(SimpleVector&& other){
        (*this).swap(other);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if(&array_ != &rhs.array_){
            SimpleVector vector_tmp(rhs);
            (*this).swap(vector_tmp);
        }
        return *this;
    }
    
    SimpleVector& operator=(SimpleVector&& rhs) {
        if(&array_ != &rhs.array_){
            (*this).swap(rhs);
        }
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        size_t size_tmp = size_;
        if(size_ == capacity_){
            this->Resize(2*capacity_ + 1);
        } 
        this->array_[size_tmp] = item;
        this->size_ = std::move(size_tmp + 1);
    }

    void PushBack(Type&& item) {
        size_t size_tmp = size_;
        if(size_ == capacity_){
            this->Resize(2*capacity_ + 1);
        } 
        this->array_[size_tmp] = std::move(item);
        this->size_ = std::move(size_tmp + 1);
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t n = pos - begin();
        PreInsert(n);
        array_[n] = value;
        return &array_[n];
    }
    
    Iterator Insert(ConstIterator pos, Type&& value) {
        size_t n = pos - begin();
        PreInsert(n);
        array_[n] = std::move(value);
        return &array_[n];
    }
    
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack () noexcept {
        assert(!IsEmpty());
        --size_;
    }
    
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        size_t n = pos - begin();
        for(Type *i = &array_[n] ; i < &array_[size_ - 1]; ++i){
            *i = std::move(*(i + 1));   
        }
        --size_;
        return &array_[n];
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        array_.swap(other.array_);
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
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if(index >= size_){
            throw std::out_of_range("Array size exceeding");  
        } 
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if(index >= size_){
            throw std::out_of_range("Array size exceeding");  
        } 
        return array_[index]; 
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }
    
    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if(new_size <= size_) {}
        else if(new_size <= capacity_){
            Fill(begin() + size_, begin() + new_size); 
        } 
        else {
            ArrayPtr<Type> array_tmp(new_size);
            std::copy(std::make_move_iterator(begin()), std::make_move_iterator(begin() + size_), array_tmp.Get());
            Fill(array_tmp.Get() + size_, array_tmp.Get() + new_size);
            array_.swap(array_tmp);
            capacity_ = new_size;
        }
        size_ = new_size;
    }
    
    void Fill(Iterator start, Iterator stop){
        for(Iterator i = start; i != stop; ++i){
            *i = Type{};
        } 
    }
    
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return array_.Get() + size_;
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return array_.Get();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return array_.Get() + size_;
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
private:
    size_t size_ = 0;
    size_t capacity_ = 0;
    ArrayPtr<Type> array_;

    void PreInsert(const size_t& n){
        size_t size_tmp = size_;
        if(size_ == capacity_){
            this->Resize(2*capacity_ + 1);  
        }  
        for(Type *i = &array_[size_tmp-1]; i >= &array_[n]; --i){
            *(i + 1) = std::move(*i);
        } 
        size_ = std::move(size_tmp + 1);
    }

};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
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
    return !(lhs < rhs);
} 
