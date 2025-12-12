#pragma once

#include <cstdint>
#include <cstddef>
#include <array>
#include <vector>
#include <type_traits>
#include <concepts>

namespace truk::aether {

template<typename T>
concept numeric = std::integral<T> || std::floating_point<T>;

struct dynamic_t {
    std::vector<std::uint8_t> bytes;

    dynamic_t() = default;
    explicit dynamic_t(std::vector<std::uint8_t> val) : bytes(std::move(val)) {}
    dynamic_t(const std::uint8_t* data, std::size_t size) : bytes(data, data + size) {}
};

template<typename T>
concept monad_type = numeric<T> || std::same_as<T, dynamic_t>;

template<monad_type T>
class monad_c {
public:
    monad_c() = delete;
    constexpr monad_c(T val) : _data(std::move(val)){}
    constexpr ~monad_c() = default;
protected:
    constexpr std::size_t this_monad_width() const noexcept {
        if constexpr (std::same_as<T, dynamic_t>) {
            return _data.bytes.size();
        } else {
            return sizeof(T);
        }
    }
    constexpr const T& data() const noexcept { return _data; }
    constexpr void set_data(T val) noexcept { _data = std::move(val); }
    constexpr T& data_ref() noexcept { return _data; }
    constexpr const T& data_ref() const noexcept { return _data; }
private:
    T _data;
};

template<numeric T>
class numeric_if {
public:
    constexpr ~numeric_if() = default;
};

class dynamic_if {
public:
    ~dynamic_if() = default;
};

template<std::integral T>
class integer_base_if : public monad_c<T>, public numeric_if<T> {
public:
    integer_base_if() = delete;
    constexpr integer_base_if(T val) : monad_c<T>(val) {}
    constexpr ~integer_base_if() = default;
};

template<std::floating_point T>
class real_base_if : public monad_c<T>, public numeric_if<T> {
public:
    real_base_if() = delete;
    constexpr real_base_if(T val) : monad_c<T>(val) {}
    constexpr ~real_base_if() = default;
};

class dynamic_base_if : public monad_c<dynamic_t>, public dynamic_if {
public:
    dynamic_base_if() = delete;
    explicit dynamic_base_if(dynamic_t val) : monad_c<dynamic_t>(std::move(val)) {}
    explicit dynamic_base_if(std::vector<std::uint8_t> val) : monad_c<dynamic_t>(dynamic_t(std::move(val))) {}
    dynamic_base_if(const std::uint8_t* data, std::size_t size) : monad_c<dynamic_t>(dynamic_t(data, size)) {}
    ~dynamic_base_if() = default;
};

class i8_c : public integer_base_if<std::uint8_t> {
public:
    i8_c() = delete;
    constexpr i8_c(std::uint8_t value) : integer_base_if<std::uint8_t>(value){}
    constexpr ~i8_c() = default;

    constexpr i8_c operator+(const i8_c& other) const noexcept { return i8_c(data() + other.data()); }
    constexpr i8_c operator-(const i8_c& other) const noexcept { return i8_c(data() - other.data()); }
    constexpr i8_c operator*(const i8_c& other) const noexcept { return i8_c(data() * other.data()); }
    constexpr i8_c operator/(const i8_c& other) const noexcept { return i8_c(data() / other.data()); }
    constexpr i8_c operator%(const i8_c& other) const noexcept { return i8_c(data() % other.data()); }
    
    constexpr i8_c& operator+=(const i8_c& other) noexcept { set_data(data() + other.data()); return *this; }
    constexpr i8_c& operator-=(const i8_c& other) noexcept { set_data(data() - other.data()); return *this; }
    constexpr i8_c& operator*=(const i8_c& other) noexcept { set_data(data() * other.data()); return *this; }
    constexpr i8_c& operator/=(const i8_c& other) noexcept { set_data(data() / other.data()); return *this; }
    constexpr i8_c& operator%=(const i8_c& other) noexcept { set_data(data() % other.data()); return *this; }
    
    constexpr bool operator==(const i8_c& other) const noexcept { return data() == other.data(); }
    constexpr bool operator!=(const i8_c& other) const noexcept { return data() != other.data(); }
    constexpr bool operator<(const i8_c& other) const noexcept { return data() < other.data(); }
    constexpr bool operator<=(const i8_c& other) const noexcept { return data() <= other.data(); }
    constexpr bool operator>(const i8_c& other) const noexcept { return data() > other.data(); }
    constexpr bool operator>=(const i8_c& other) const noexcept { return data() >= other.data(); }
    
    constexpr i8_c& operator++() noexcept { set_data(data() + 1); return *this; }
    constexpr i8_c operator++(int) noexcept { i8_c tmp(*this); ++(*this); return tmp; }
    constexpr i8_c& operator--() noexcept { set_data(data() - 1); return *this; }
    constexpr i8_c operator--(int) noexcept { i8_c tmp(*this); --(*this); return tmp; }
};

class i16_c : public integer_base_if<std::uint16_t> {
public:
    i16_c() = delete;
    constexpr i16_c(std::uint16_t value) : integer_base_if<std::uint16_t>(value){}
    constexpr ~i16_c() = default;

    constexpr i16_c operator+(const i16_c& other) const noexcept { return i16_c(data() + other.data()); }
    constexpr i16_c operator-(const i16_c& other) const noexcept { return i16_c(data() - other.data()); }
    constexpr i16_c operator*(const i16_c& other) const noexcept { return i16_c(data() * other.data()); }
    constexpr i16_c operator/(const i16_c& other) const noexcept { return i16_c(data() / other.data()); }
    constexpr i16_c operator%(const i16_c& other) const noexcept { return i16_c(data() % other.data()); }
    
    constexpr i16_c& operator+=(const i16_c& other) noexcept { set_data(data() + other.data()); return *this; }
    constexpr i16_c& operator-=(const i16_c& other) noexcept { set_data(data() - other.data()); return *this; }
    constexpr i16_c& operator*=(const i16_c& other) noexcept { set_data(data() * other.data()); return *this; }
    constexpr i16_c& operator/=(const i16_c& other) noexcept { set_data(data() / other.data()); return *this; }
    constexpr i16_c& operator%=(const i16_c& other) noexcept { set_data(data() % other.data()); return *this; }
    
    constexpr bool operator==(const i16_c& other) const noexcept { return data() == other.data(); }
    constexpr bool operator!=(const i16_c& other) const noexcept { return data() != other.data(); }
    constexpr bool operator<(const i16_c& other) const noexcept { return data() < other.data(); }
    constexpr bool operator<=(const i16_c& other) const noexcept { return data() <= other.data(); }
    constexpr bool operator>(const i16_c& other) const noexcept { return data() > other.data(); }
    constexpr bool operator>=(const i16_c& other) const noexcept { return data() >= other.data(); }
    
    constexpr i16_c& operator++() noexcept { set_data(data() + 1); return *this; }
    constexpr i16_c operator++(int) noexcept { i16_c tmp(*this); ++(*this); return tmp; }
    constexpr i16_c& operator--() noexcept { set_data(data() - 1); return *this; }
    constexpr i16_c operator--(int) noexcept { i16_c tmp(*this); --(*this); return tmp; }
};

class i32_c : public integer_base_if<std::uint32_t> {
public:
    i32_c() = delete;
    constexpr i32_c(std::uint32_t value) : integer_base_if<std::uint32_t>(value){}
    constexpr ~i32_c() = default;

    constexpr i32_c operator+(const i32_c& other) const noexcept { return i32_c(data() + other.data()); }
    constexpr i32_c operator-(const i32_c& other) const noexcept { return i32_c(data() - other.data()); }
    constexpr i32_c operator*(const i32_c& other) const noexcept { return i32_c(data() * other.data()); }
    constexpr i32_c operator/(const i32_c& other) const noexcept { return i32_c(data() / other.data()); }
    constexpr i32_c operator%(const i32_c& other) const noexcept { return i32_c(data() % other.data()); }
    
    constexpr i32_c& operator+=(const i32_c& other) noexcept { set_data(data() + other.data()); return *this; }
    constexpr i32_c& operator-=(const i32_c& other) noexcept { set_data(data() - other.data()); return *this; }
    constexpr i32_c& operator*=(const i32_c& other) noexcept { set_data(data() * other.data()); return *this; }
    constexpr i32_c& operator/=(const i32_c& other) noexcept { set_data(data() / other.data()); return *this; }
    constexpr i32_c& operator%=(const i32_c& other) noexcept { set_data(data() % other.data()); return *this; }
    
    constexpr bool operator==(const i32_c& other) const noexcept { return data() == other.data(); }
    constexpr bool operator!=(const i32_c& other) const noexcept { return data() != other.data(); }
    constexpr bool operator<(const i32_c& other) const noexcept { return data() < other.data(); }
    constexpr bool operator<=(const i32_c& other) const noexcept { return data() <= other.data(); }
    constexpr bool operator>(const i32_c& other) const noexcept { return data() > other.data(); }
    constexpr bool operator>=(const i32_c& other) const noexcept { return data() >= other.data(); }
    
    constexpr i32_c& operator++() noexcept { set_data(data() + 1); return *this; }
    constexpr i32_c operator++(int) noexcept { i32_c tmp(*this); ++(*this); return tmp; }
    constexpr i32_c& operator--() noexcept { set_data(data() - 1); return *this; }
    constexpr i32_c operator--(int) noexcept { i32_c tmp(*this); --(*this); return tmp; }
};

class i64_c : public integer_base_if<std::uint64_t> {
public:
    i64_c() = delete;
    constexpr i64_c(std::uint64_t value) : integer_base_if<std::uint64_t>(value){}
    constexpr ~i64_c() = default;

    constexpr i64_c operator+(const i64_c& other) const noexcept { return i64_c(data() + other.data()); }
    constexpr i64_c operator-(const i64_c& other) const noexcept { return i64_c(data() - other.data()); }
    constexpr i64_c operator*(const i64_c& other) const noexcept { return i64_c(data() * other.data()); }
    constexpr i64_c operator/(const i64_c& other) const noexcept { return i64_c(data() / other.data()); }
    constexpr i64_c operator%(const i64_c& other) const noexcept { return i64_c(data() % other.data()); }
    
    constexpr i64_c& operator+=(const i64_c& other) noexcept { set_data(data() + other.data()); return *this; }
    constexpr i64_c& operator-=(const i64_c& other) noexcept { set_data(data() - other.data()); return *this; }
    constexpr i64_c& operator*=(const i64_c& other) noexcept { set_data(data() * other.data()); return *this; }
    constexpr i64_c& operator/=(const i64_c& other) noexcept { set_data(data() / other.data()); return *this; }
    constexpr i64_c& operator%=(const i64_c& other) noexcept { set_data(data() % other.data()); return *this; }
    
    constexpr bool operator==(const i64_c& other) const noexcept { return data() == other.data(); }
    constexpr bool operator!=(const i64_c& other) const noexcept { return data() != other.data(); }
    constexpr bool operator<(const i64_c& other) const noexcept { return data() < other.data(); }
    constexpr bool operator<=(const i64_c& other) const noexcept { return data() <= other.data(); }
    constexpr bool operator>(const i64_c& other) const noexcept { return data() > other.data(); }
    constexpr bool operator>=(const i64_c& other) const noexcept { return data() >= other.data(); }
    
    constexpr i64_c& operator++() noexcept { set_data(data() + 1); return *this; }
    constexpr i64_c operator++(int) noexcept { i64_c tmp(*this); ++(*this); return tmp; }
    constexpr i64_c& operator--() noexcept { set_data(data() - 1); return *this; }
    constexpr i64_c operator--(int) noexcept { i64_c tmp(*this); --(*this); return tmp; }
};

class r32_c : public real_base_if<float> {
public:
    r32_c() = delete;
    constexpr r32_c(float value) : real_base_if<float>(value){}
    constexpr ~r32_c() = default;

    constexpr r32_c operator+(const r32_c& other) const noexcept { return r32_c(data() + other.data()); }
    constexpr r32_c operator-(const r32_c& other) const noexcept { return r32_c(data() - other.data()); }
    constexpr r32_c operator*(const r32_c& other) const noexcept { return r32_c(data() * other.data()); }
    constexpr r32_c operator/(const r32_c& other) const noexcept { return r32_c(data() / other.data()); }
    
    constexpr r32_c& operator+=(const r32_c& other) noexcept { set_data(data() + other.data()); return *this; }
    constexpr r32_c& operator-=(const r32_c& other) noexcept { set_data(data() - other.data()); return *this; }
    constexpr r32_c& operator*=(const r32_c& other) noexcept { set_data(data() * other.data()); return *this; }
    constexpr r32_c& operator/=(const r32_c& other) noexcept { set_data(data() / other.data()); return *this; }
    
    constexpr bool operator==(const r32_c& other) const noexcept { return data() == other.data(); }
    constexpr bool operator!=(const r32_c& other) const noexcept { return data() != other.data(); }
    constexpr bool operator<(const r32_c& other) const noexcept { return data() < other.data(); }
    constexpr bool operator<=(const r32_c& other) const noexcept { return data() <= other.data(); }
    constexpr bool operator>(const r32_c& other) const noexcept { return data() > other.data(); }
    constexpr bool operator>=(const r32_c& other) const noexcept { return data() >= other.data(); }
    
    constexpr r32_c& operator++() noexcept { set_data(data() + 1.0f); return *this; }
    constexpr r32_c operator++(int) noexcept { r32_c tmp(*this); ++(*this); return tmp; }
    constexpr r32_c& operator--() noexcept { set_data(data() - 1.0f); return *this; }
    constexpr r32_c operator--(int) noexcept { r32_c tmp(*this); --(*this); return tmp; }
};

class r64_c : public real_base_if<double> {
public:
    r64_c() = delete;
    constexpr r64_c(double value) : real_base_if<double>(value){}
    constexpr ~r64_c() = default;

    constexpr r64_c operator+(const r64_c& other) const noexcept { return r64_c(data() + other.data()); }
    constexpr r64_c operator-(const r64_c& other) const noexcept { return r64_c(data() - other.data()); }
    constexpr r64_c operator*(const r64_c& other) const noexcept { return r64_c(data() * other.data()); }
    constexpr r64_c operator/(const r64_c& other) const noexcept { return r64_c(data() / other.data()); }
    
    constexpr r64_c& operator+=(const r64_c& other) noexcept { set_data(data() + other.data()); return *this; }
    constexpr r64_c& operator-=(const r64_c& other) noexcept { set_data(data() - other.data()); return *this; }
    constexpr r64_c& operator*=(const r64_c& other) noexcept { set_data(data() * other.data()); return *this; }
    constexpr r64_c& operator/=(const r64_c& other) noexcept { set_data(data() / other.data()); return *this; }
    
    constexpr bool operator==(const r64_c& other) const noexcept { return data() == other.data(); }
    constexpr bool operator!=(const r64_c& other) const noexcept { return data() != other.data(); }
    constexpr bool operator<(const r64_c& other) const noexcept { return data() < other.data(); }
    constexpr bool operator<=(const r64_c& other) const noexcept { return data() <= other.data(); }
    constexpr bool operator>(const r64_c& other) const noexcept { return data() > other.data(); }
    constexpr bool operator>=(const r64_c& other) const noexcept { return data() >= other.data(); }
    
    constexpr r64_c& operator++() noexcept { set_data(data() + 1.0); return *this; }
    constexpr r64_c operator++(int) noexcept { r64_c tmp(*this); ++(*this); return tmp; }
    constexpr r64_c& operator--() noexcept { set_data(data() - 1.0); return *this; }
    constexpr r64_c operator--(int) noexcept { r64_c tmp(*this); --(*this); return tmp; }
};


} // namespace