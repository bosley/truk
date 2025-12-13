#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <memory>
#include <new>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace truk::aether {

class aether_bounds_exception_c : public std::exception {
private:
  std::string _message;

public:
  aether_bounds_exception_c(std::size_t index, std::size_t length) {
    std::ostringstream oss;
    oss << "Array index out of bounds: index " << index << " >= length "
        << length;
    _message = oss.str();
  }

  const char *what() const noexcept override { return _message.c_str(); }
};

template <typename T>
concept numeric = std::integral<T> || std::floating_point<T>;

template <numeric T>
inline void write_little_endian(std::uint8_t *dest, T value) noexcept {
  if constexpr (sizeof(T) == 1) {
    std::memcpy(dest, &value, 1);
  } else if constexpr (std::is_floating_point_v<T>) {
    typename std::conditional<sizeof(T) == 4, std::uint32_t,
                              std::uint64_t>::type bits;
    std::memcpy(&bits, &value, sizeof(T));
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      dest[i] = static_cast<std::uint8_t>((bits >> (i * 8)) & 0xFF);
    }
  } else {
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      dest[i] = static_cast<std::uint8_t>((value >> (i * 8)) & 0xFF);
    }
  }
}

template <numeric T>
inline void write_big_endian(std::uint8_t *dest, T value) noexcept {
  if constexpr (sizeof(T) == 1) {
    std::memcpy(dest, &value, 1);
  } else if constexpr (std::is_floating_point_v<T>) {
    typename std::conditional<sizeof(T) == 4, std::uint32_t,
                              std::uint64_t>::type bits;
    std::memcpy(&bits, &value, sizeof(T));
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      dest[sizeof(T) - 1 - i] =
          static_cast<std::uint8_t>((bits >> (i * 8)) & 0xFF);
    }
  } else {
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      dest[sizeof(T) - 1 - i] =
          static_cast<std::uint8_t>((value >> (i * 8)) & 0xFF);
    }
  }
}

template <numeric T>
inline T read_little_endian(const std::uint8_t *src) noexcept {
  if constexpr (sizeof(T) == 1) {
    T result;
    std::memcpy(&result, src, 1);
    return result;
  } else if constexpr (std::is_floating_point_v<T>) {
    typename std::conditional<sizeof(T) == 4, std::uint32_t,
                              std::uint64_t>::type bits = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      bits |= static_cast<decltype(bits)>(src[i]) << (i * 8);
    }
    T result;
    std::memcpy(&result, &bits, sizeof(T));
    return result;
  } else {
    T result = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      result |= static_cast<T>(src[i]) << (i * 8);
    }
    return result;
  }
}

template <numeric T>
inline T read_big_endian(const std::uint8_t *src) noexcept {
  if constexpr (sizeof(T) == 1) {
    T result;
    std::memcpy(&result, src, 1);
    return result;
  } else if constexpr (std::is_floating_point_v<T>) {
    typename std::conditional<sizeof(T) == 4, std::uint32_t,
                              std::uint64_t>::type bits = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      bits |= static_cast<decltype(bits)>(src[sizeof(T) - 1 - i]) << (i * 8);
    }
    T result;
    std::memcpy(&result, &bits, sizeof(T));
    return result;
  } else {
    T result = 0;
    for (std::size_t i = 0; i < sizeof(T); ++i) {
      result |= static_cast<T>(src[sizeof(T) - 1 - i]) << (i * 8);
    }
    return result;
  }
}

struct dynamic_t {
  std::vector<std::uint8_t> bytes;

  dynamic_t() = default;
  explicit dynamic_t(std::vector<std::uint8_t> val) : bytes(std::move(val)) {}
  dynamic_t(const std::uint8_t *data, std::size_t size)
      : bytes(data, data + size) {}

  void append_bytes(const std::uint8_t *data, std::size_t size) {
    bytes.insert(bytes.end(), data, data + size);
  }

  void append_byte(std::uint8_t byte) { bytes.push_back(byte); }

  template <numeric T> void pack(T value, bool little_endian = true) {
    std::size_t old_size = bytes.size();
    bytes.resize(old_size + sizeof(T));
    if (little_endian) {
      write_little_endian<T>(bytes.data() + old_size, value);
    } else {
      write_big_endian<T>(bytes.data() + old_size, value);
    }
  }

  template <numeric T>
  T unpack(std::size_t offset, bool little_endian = true) const {
    if (offset + sizeof(T) > bytes.size()) {
      return T{};
    }
    if (little_endian) {
      return read_little_endian<T>(bytes.data() + offset);
    } else {
      return read_big_endian<T>(bytes.data() + offset);
    }
  }

  std::uint8_t at(std::size_t index) const { return bytes.at(index); }

  std::size_t size() const noexcept { return bytes.size(); }

  void clear() { bytes.clear(); }
};

template <typename T>
concept monad_type = numeric<T> || std::same_as<T, dynamic_t>;

template <monad_type T> class monad_c {
public:
  monad_c() = delete;
  constexpr monad_c(T val) : _data(std::move(val)) {}
  constexpr ~monad_c() = default;

protected:
  constexpr std::size_t this_monad_width() const noexcept {
    if constexpr (std::same_as<T, dynamic_t>) {
      return _data.bytes.size();
    } else {
      return sizeof(T);
    }
  }
  constexpr const T &data() const noexcept { return _data; }
  constexpr void set_data(T val) noexcept { _data = std::move(val); }
  constexpr T &data_ref() noexcept { return _data; }
  constexpr const T &data_ref() const noexcept { return _data; }

private:
  T _data;
};

template <numeric T> class numeric_if {
public:
  constexpr ~numeric_if() = default;
};

class dynamic_if {
public:
  ~dynamic_if() = default;
};

template <std::integral T>
class integer_base_if : public monad_c<T>, public numeric_if<T> {
public:
  integer_base_if() = delete;
  constexpr integer_base_if(T val) : monad_c<T>(val) {}
  constexpr ~integer_base_if() = default;

  constexpr T value() const noexcept { return this->data(); }
};

template <std::floating_point T>
class real_base_if : public monad_c<T>, public numeric_if<T> {
public:
  real_base_if() = delete;
  constexpr real_base_if(T val) : monad_c<T>(val) {}
  constexpr ~real_base_if() = default;

  constexpr T value() const noexcept { return this->data(); }
};

class dynamic_base_if : public monad_c<dynamic_t>, public dynamic_if {
public:
  dynamic_base_if() = delete;
  explicit dynamic_base_if(dynamic_t val)
      : monad_c<dynamic_t>(std::move(val)) {}
  explicit dynamic_base_if(std::vector<std::uint8_t> val)
      : monad_c<dynamic_t>(dynamic_t(std::move(val))) {}
  dynamic_base_if(const std::uint8_t *data, std::size_t size)
      : monad_c<dynamic_t>(dynamic_t(data, size)) {}
  ~dynamic_base_if() = default;

  template <numeric T> void pack_value(T value, bool little_endian = true) {
    this->data_ref().pack(value, little_endian);
  }

  template <numeric T>
  T unpack_value(std::size_t offset, bool little_endian = true) const {
    return this->data().unpack<T>(offset, little_endian);
  }

  std::size_t byte_size() const noexcept { return this->data().size(); }

  const std::vector<std::uint8_t> &get_bytes() const noexcept {
    return this->data().bytes;
  }
};

class string_c : public dynamic_base_if {
public:
  string_c() : dynamic_base_if(dynamic_t()) { ensure_null_terminated(); }

  explicit string_c(const char *str)
      : dynamic_base_if(dynamic_t(reinterpret_cast<const std::uint8_t *>(str),
                                  str ? std::strlen(str) : 0)) {
    ensure_null_terminated();
  }

  explicit string_c(const std::string &str)
      : dynamic_base_if(dynamic_t(
            reinterpret_cast<const std::uint8_t *>(str.data()), str.size())) {
    ensure_null_terminated();
  }

  explicit string_c(std::vector<std::uint8_t> bytes)
      : dynamic_base_if(dynamic_t(std::move(bytes))) {
    ensure_null_terminated();
  }

  const char *c_str() const noexcept {
    return reinterpret_cast<const char *>(get_bytes().data());
  }

  std::string to_string() const {
    const auto &bytes = get_bytes();
    if (bytes.empty() || bytes.back() != 0) {
      return std::string();
    }
    return std::string(reinterpret_cast<const char *>(bytes.data()),
                       bytes.size() - 1);
  }

  std::size_t string_length() const noexcept {
    std::size_t size = byte_size();
    return size > 0 ? size - 1 : 0;
  }

  void append(const char *str) {
    if (!str)
      return;
    remove_null_terminator();
    std::size_t len = std::strlen(str);
    const std::uint8_t *bytes = reinterpret_cast<const std::uint8_t *>(str);
    for (std::size_t i = 0; i < len; ++i) {
      this->data_ref().append_byte(bytes[i]);
    }
    ensure_null_terminated();
  }

  void append(const std::string &str) {
    remove_null_terminator();
    const std::uint8_t *bytes =
        reinterpret_cast<const std::uint8_t *>(str.data());
    for (std::size_t i = 0; i < str.size(); ++i) {
      this->data_ref().append_byte(bytes[i]);
    }
    ensure_null_terminated();
  }

  void clear() {
    this->data_ref().clear();
    ensure_null_terminated();
  }

private:
  void ensure_null_terminated() {
    auto &bytes = this->data_ref().bytes;
    if (bytes.empty() || bytes.back() != 0) {
      bytes.push_back(0);
    }
  }

  void remove_null_terminator() {
    auto &bytes = this->data_ref().bytes;
    if (!bytes.empty() && bytes.back() == 0) {
      bytes.pop_back();
    }
  }
};

class i8_c : public integer_base_if<std::int8_t> {
public:
  i8_c() = delete;
  constexpr i8_c(std::int8_t value) : integer_base_if<std::int8_t>(value) {}
  constexpr ~i8_c() = default;

  constexpr i8_c operator+(const i8_c &other) const noexcept {
    return i8_c(data() + other.data());
  }
  constexpr i8_c operator-(const i8_c &other) const noexcept {
    return i8_c(data() - other.data());
  }
  constexpr i8_c operator*(const i8_c &other) const noexcept {
    return i8_c(data() * other.data());
  }
  constexpr i8_c operator/(const i8_c &other) const noexcept {
    return i8_c(data() / other.data());
  }
  constexpr i8_c operator%(const i8_c &other) const noexcept {
    return i8_c(data() % other.data());
  }

  constexpr i8_c &operator+=(const i8_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr i8_c &operator-=(const i8_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr i8_c &operator*=(const i8_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr i8_c &operator/=(const i8_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr i8_c &operator%=(const i8_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const i8_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const i8_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const i8_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const i8_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const i8_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const i8_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr i8_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr i8_c operator++(int) noexcept {
    i8_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr i8_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr i8_c operator--(int) noexcept {
    i8_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class u8_c : public integer_base_if<std::uint8_t> {
public:
  u8_c() = delete;
  constexpr u8_c(std::uint8_t value) : integer_base_if<std::uint8_t>(value) {}
  constexpr ~u8_c() = default;

  constexpr u8_c operator+(const u8_c &other) const noexcept {
    return u8_c(data() + other.data());
  }
  constexpr u8_c operator-(const u8_c &other) const noexcept {
    return u8_c(data() - other.data());
  }
  constexpr u8_c operator*(const u8_c &other) const noexcept {
    return u8_c(data() * other.data());
  }
  constexpr u8_c operator/(const u8_c &other) const noexcept {
    return u8_c(data() / other.data());
  }
  constexpr u8_c operator%(const u8_c &other) const noexcept {
    return u8_c(data() % other.data());
  }

  constexpr u8_c &operator+=(const u8_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr u8_c &operator-=(const u8_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr u8_c &operator*=(const u8_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr u8_c &operator/=(const u8_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr u8_c &operator%=(const u8_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const u8_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const u8_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const u8_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const u8_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const u8_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const u8_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr u8_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr u8_c operator++(int) noexcept {
    u8_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr u8_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr u8_c operator--(int) noexcept {
    u8_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class i16_c : public integer_base_if<std::int16_t> {
public:
  i16_c() = delete;
  constexpr i16_c(std::int16_t value) : integer_base_if<std::int16_t>(value) {}
  constexpr ~i16_c() = default;

  constexpr i16_c operator+(const i16_c &other) const noexcept {
    return i16_c(data() + other.data());
  }
  constexpr i16_c operator-(const i16_c &other) const noexcept {
    return i16_c(data() - other.data());
  }
  constexpr i16_c operator*(const i16_c &other) const noexcept {
    return i16_c(data() * other.data());
  }
  constexpr i16_c operator/(const i16_c &other) const noexcept {
    return i16_c(data() / other.data());
  }
  constexpr i16_c operator%(const i16_c &other) const noexcept {
    return i16_c(data() % other.data());
  }

  constexpr i16_c &operator+=(const i16_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr i16_c &operator-=(const i16_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr i16_c &operator*=(const i16_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr i16_c &operator/=(const i16_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr i16_c &operator%=(const i16_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const i16_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const i16_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const i16_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const i16_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const i16_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const i16_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr i16_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr i16_c operator++(int) noexcept {
    i16_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr i16_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr i16_c operator--(int) noexcept {
    i16_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class u16_c : public integer_base_if<std::uint16_t> {
public:
  u16_c() = delete;
  constexpr u16_c(std::uint16_t value)
      : integer_base_if<std::uint16_t>(value) {}
  constexpr ~u16_c() = default;

  constexpr u16_c operator+(const u16_c &other) const noexcept {
    return u16_c(data() + other.data());
  }
  constexpr u16_c operator-(const u16_c &other) const noexcept {
    return u16_c(data() - other.data());
  }
  constexpr u16_c operator*(const u16_c &other) const noexcept {
    return u16_c(data() * other.data());
  }
  constexpr u16_c operator/(const u16_c &other) const noexcept {
    return u16_c(data() / other.data());
  }
  constexpr u16_c operator%(const u16_c &other) const noexcept {
    return u16_c(data() % other.data());
  }

  constexpr u16_c &operator+=(const u16_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr u16_c &operator-=(const u16_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr u16_c &operator*=(const u16_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr u16_c &operator/=(const u16_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr u16_c &operator%=(const u16_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const u16_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const u16_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const u16_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const u16_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const u16_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const u16_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr u16_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr u16_c operator++(int) noexcept {
    u16_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr u16_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr u16_c operator--(int) noexcept {
    u16_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class i32_c : public integer_base_if<std::int32_t> {
public:
  i32_c() = delete;
  constexpr i32_c(std::int32_t value) : integer_base_if<std::int32_t>(value) {}
  constexpr ~i32_c() = default;

  constexpr i32_c operator+(const i32_c &other) const noexcept {
    return i32_c(data() + other.data());
  }
  constexpr i32_c operator-(const i32_c &other) const noexcept {
    return i32_c(data() - other.data());
  }
  constexpr i32_c operator*(const i32_c &other) const noexcept {
    return i32_c(data() * other.data());
  }
  constexpr i32_c operator/(const i32_c &other) const noexcept {
    return i32_c(data() / other.data());
  }
  constexpr i32_c operator%(const i32_c &other) const noexcept {
    return i32_c(data() % other.data());
  }

  constexpr i32_c &operator+=(const i32_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr i32_c &operator-=(const i32_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr i32_c &operator*=(const i32_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr i32_c &operator/=(const i32_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr i32_c &operator%=(const i32_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const i32_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const i32_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const i32_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const i32_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const i32_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const i32_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr i32_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr i32_c operator++(int) noexcept {
    i32_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr i32_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr i32_c operator--(int) noexcept {
    i32_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class u32_c : public integer_base_if<std::uint32_t> {
public:
  u32_c() = delete;
  constexpr u32_c(std::uint32_t value)
      : integer_base_if<std::uint32_t>(value) {}
  constexpr ~u32_c() = default;

  constexpr u32_c operator+(const u32_c &other) const noexcept {
    return u32_c(data() + other.data());
  }
  constexpr u32_c operator-(const u32_c &other) const noexcept {
    return u32_c(data() - other.data());
  }
  constexpr u32_c operator*(const u32_c &other) const noexcept {
    return u32_c(data() * other.data());
  }
  constexpr u32_c operator/(const u32_c &other) const noexcept {
    return u32_c(data() / other.data());
  }
  constexpr u32_c operator%(const u32_c &other) const noexcept {
    return u32_c(data() % other.data());
  }

  constexpr u32_c &operator+=(const u32_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr u32_c &operator-=(const u32_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr u32_c &operator*=(const u32_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr u32_c &operator/=(const u32_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr u32_c &operator%=(const u32_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const u32_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const u32_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const u32_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const u32_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const u32_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const u32_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr u32_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr u32_c operator++(int) noexcept {
    u32_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr u32_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr u32_c operator--(int) noexcept {
    u32_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class i64_c : public integer_base_if<std::int64_t> {
public:
  i64_c() = delete;
  constexpr i64_c(std::int64_t value) : integer_base_if<std::int64_t>(value) {}
  constexpr ~i64_c() = default;

  constexpr i64_c operator+(const i64_c &other) const noexcept {
    return i64_c(data() + other.data());
  }
  constexpr i64_c operator-(const i64_c &other) const noexcept {
    return i64_c(data() - other.data());
  }
  constexpr i64_c operator*(const i64_c &other) const noexcept {
    return i64_c(data() * other.data());
  }
  constexpr i64_c operator/(const i64_c &other) const noexcept {
    return i64_c(data() / other.data());
  }
  constexpr i64_c operator%(const i64_c &other) const noexcept {
    return i64_c(data() % other.data());
  }

  constexpr i64_c &operator+=(const i64_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr i64_c &operator-=(const i64_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr i64_c &operator*=(const i64_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr i64_c &operator/=(const i64_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr i64_c &operator%=(const i64_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const i64_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const i64_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const i64_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const i64_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const i64_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const i64_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr i64_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr i64_c operator++(int) noexcept {
    i64_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr i64_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr i64_c operator--(int) noexcept {
    i64_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class u64_c : public integer_base_if<std::uint64_t> {
public:
  u64_c() = delete;
  constexpr u64_c(std::uint64_t value)
      : integer_base_if<std::uint64_t>(value) {}
  constexpr ~u64_c() = default;

  constexpr u64_c operator+(const u64_c &other) const noexcept {
    return u64_c(data() + other.data());
  }
  constexpr u64_c operator-(const u64_c &other) const noexcept {
    return u64_c(data() - other.data());
  }
  constexpr u64_c operator*(const u64_c &other) const noexcept {
    return u64_c(data() * other.data());
  }
  constexpr u64_c operator/(const u64_c &other) const noexcept {
    return u64_c(data() / other.data());
  }
  constexpr u64_c operator%(const u64_c &other) const noexcept {
    return u64_c(data() % other.data());
  }

  constexpr u64_c &operator+=(const u64_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr u64_c &operator-=(const u64_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr u64_c &operator*=(const u64_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr u64_c &operator/=(const u64_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }
  constexpr u64_c &operator%=(const u64_c &other) noexcept {
    set_data(data() % other.data());
    return *this;
  }

  constexpr bool operator==(const u64_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const u64_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const u64_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const u64_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const u64_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const u64_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr u64_c &operator++() noexcept {
    set_data(data() + 1);
    return *this;
  }
  constexpr u64_c operator++(int) noexcept {
    u64_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr u64_c &operator--() noexcept {
    set_data(data() - 1);
    return *this;
  }
  constexpr u64_c operator--(int) noexcept {
    u64_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class r32_c : public real_base_if<float> {
public:
  r32_c() = delete;
  constexpr r32_c(float value) : real_base_if<float>(value) {}
  constexpr ~r32_c() = default;

  constexpr r32_c operator+(const r32_c &other) const noexcept {
    return r32_c(data() + other.data());
  }
  constexpr r32_c operator-(const r32_c &other) const noexcept {
    return r32_c(data() - other.data());
  }
  constexpr r32_c operator*(const r32_c &other) const noexcept {
    return r32_c(data() * other.data());
  }
  constexpr r32_c operator/(const r32_c &other) const noexcept {
    return r32_c(data() / other.data());
  }

  constexpr r32_c &operator+=(const r32_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr r32_c &operator-=(const r32_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr r32_c &operator*=(const r32_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr r32_c &operator/=(const r32_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }

  constexpr bool operator==(const r32_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const r32_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const r32_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const r32_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const r32_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const r32_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr r32_c &operator++() noexcept {
    set_data(data() + 1.0f);
    return *this;
  }
  constexpr r32_c operator++(int) noexcept {
    r32_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr r32_c &operator--() noexcept {
    set_data(data() - 1.0f);
    return *this;
  }
  constexpr r32_c operator--(int) noexcept {
    r32_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class r64_c : public real_base_if<double> {
public:
  r64_c() = delete;
  constexpr r64_c(double value) : real_base_if<double>(value) {}
  constexpr ~r64_c() = default;

  constexpr r64_c operator+(const r64_c &other) const noexcept {
    return r64_c(data() + other.data());
  }
  constexpr r64_c operator-(const r64_c &other) const noexcept {
    return r64_c(data() - other.data());
  }
  constexpr r64_c operator*(const r64_c &other) const noexcept {
    return r64_c(data() * other.data());
  }
  constexpr r64_c operator/(const r64_c &other) const noexcept {
    return r64_c(data() / other.data());
  }

  constexpr r64_c &operator+=(const r64_c &other) noexcept {
    set_data(data() + other.data());
    return *this;
  }
  constexpr r64_c &operator-=(const r64_c &other) noexcept {
    set_data(data() - other.data());
    return *this;
  }
  constexpr r64_c &operator*=(const r64_c &other) noexcept {
    set_data(data() * other.data());
    return *this;
  }
  constexpr r64_c &operator/=(const r64_c &other) noexcept {
    set_data(data() / other.data());
    return *this;
  }

  constexpr bool operator==(const r64_c &other) const noexcept {
    return data() == other.data();
  }
  constexpr bool operator!=(const r64_c &other) const noexcept {
    return data() != other.data();
  }
  constexpr bool operator<(const r64_c &other) const noexcept {
    return data() < other.data();
  }
  constexpr bool operator<=(const r64_c &other) const noexcept {
    return data() <= other.data();
  }
  constexpr bool operator>(const r64_c &other) const noexcept {
    return data() > other.data();
  }
  constexpr bool operator>=(const r64_c &other) const noexcept {
    return data() >= other.data();
  }

  constexpr r64_c &operator++() noexcept {
    set_data(data() + 1.0);
    return *this;
  }
  constexpr r64_c operator++(int) noexcept {
    r64_c tmp(*this);
    ++(*this);
    return tmp;
  }
  constexpr r64_c &operator--() noexcept {
    set_data(data() - 1.0);
    return *this;
  }
  constexpr r64_c operator--(int) noexcept {
    r64_c tmp(*this);
    --(*this);
    return tmp;
  }
};

class bool_c : public monad_c<bool>, public numeric_if<bool> {
public:
  bool_c() = delete;
  constexpr bool_c(bool value) : monad_c<bool>(value) {}
  constexpr ~bool_c() = default;

  constexpr bool value() const noexcept { return this->data(); }

  constexpr bool_c operator&&(const bool_c &other) const noexcept {
    return bool_c(data() && other.data());
  }
  constexpr bool_c operator||(const bool_c &other) const noexcept {
    return bool_c(data() || other.data());
  }
  constexpr bool_c operator!() const noexcept { return bool_c(!data()); }
  constexpr bool operator==(const bool_c &other) const noexcept {
    return data() == other.data();
  }
};

class struct_if {
public:
  virtual ~struct_if() = default;
  virtual std::size_t size_bytes() const noexcept = 0;
  virtual std::size_t alignment() const noexcept = 0;
  virtual void *data_ptr() noexcept = 0;
  virtual const void *data_ptr() const noexcept = 0;
};

template <typename T> class struct_c : public struct_if {
private:
  using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
  storage_t _storage;
  bool _constructed;

public:
  struct_c() : _constructed(false) {
    new (&_storage) T();
    _constructed = true;
  }

  template <typename... Args>
  explicit struct_c(Args &&...args) : _constructed(false) {
    new (&_storage) T(std::forward<Args>(args)...);
    _constructed = true;
  }

  ~struct_c() {
    if (_constructed) {
      std::destroy_at(std::launder(reinterpret_cast<T *>(&_storage)));
    }
  }

  struct_c(const struct_c &) = delete;
  struct_c &operator=(const struct_c &) = delete;
  struct_c(struct_c &&) = delete;
  struct_c &operator=(struct_c &&) = delete;

  T &get() { return *std::launder(reinterpret_cast<T *>(&_storage)); }

  const T &get() const {
    return *std::launder(reinterpret_cast<const T *>(&_storage));
  }

  std::size_t size_bytes() const noexcept override { return sizeof(T); }

  std::size_t alignment() const noexcept override { return alignof(T); }

  void *data_ptr() noexcept override { return &_storage; }

  const void *data_ptr() const noexcept override { return &_storage; }
};

template <typename T>
concept array_element = (std::derived_from<T, numeric_if<std::int8_t>> ||
                         std::derived_from<T, numeric_if<std::int16_t>> ||
                         std::derived_from<T, numeric_if<std::int32_t>> ||
                         std::derived_from<T, numeric_if<std::int64_t>> ||
                         std::derived_from<T, numeric_if<std::uint8_t>> ||
                         std::derived_from<T, numeric_if<std::uint16_t>> ||
                         std::derived_from<T, numeric_if<std::uint32_t>> ||
                         std::derived_from<T, numeric_if<std::uint64_t>> ||
                         std::derived_from<T, numeric_if<float>> ||
                         std::derived_from<T, numeric_if<double>> ||
                         std::derived_from<T, numeric_if<bool>>);

template <typename T>
  requires(array_element<T> || std::derived_from<T, struct_if> ||
           std::derived_from<T, dynamic_if>)
class array_c {
private:
  using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
  std::vector<storage_t> _storage;
  std::vector<std::unique_ptr<T>> _elements;
  std::size_t _length;

public:
  explicit array_c(std::size_t length) : _length(length) {
    if constexpr (std::derived_from<T, struct_if> ||
                  std::derived_from<T, dynamic_if>) {
      _elements.reserve(length);
      for (std::size_t i = 0; i < length; ++i) {
        _elements.push_back(std::make_unique<T>());
      }
    } else {
      _storage.resize(length);
      for (std::size_t i = 0; i < _length; ++i) {
        if constexpr (std::derived_from<T, numeric_if<bool>>) {
          new (&_storage[i]) T(false);
        } else {
          new (&_storage[i]) T(0);
        }
      }
    }
  }

  ~array_c() {
    if constexpr (!std::derived_from<T, struct_if> &&
                  !std::derived_from<T, dynamic_if>) {
      for (std::size_t i = 0; i < _length; ++i) {
        std::destroy_at(std::launder(reinterpret_cast<T *>(&_storage[i])));
      }
    }
  }

  array_c(const array_c &) = delete;
  array_c &operator=(const array_c &) = delete;
  array_c(array_c &&) = delete;
  array_c &operator=(array_c &&) = delete;

  T &operator[](std::size_t idx) {
    if constexpr (std::derived_from<T, struct_if> ||
                  std::derived_from<T, dynamic_if>) {
      return *_elements[idx];
    } else {
      return *std::launder(reinterpret_cast<T *>(&_storage[idx]));
    }
  }

  const T &operator[](std::size_t idx) const {
    if constexpr (std::derived_from<T, struct_if> ||
                  std::derived_from<T, dynamic_if>) {
      return *_elements[idx];
    } else {
      return *std::launder(reinterpret_cast<const T *>(&_storage[idx]));
    }
  }

  T &at(std::size_t idx) {
    if (idx >= _length) {
      throw aether_bounds_exception_c(idx, _length);
    }
    return (*this)[idx];
  }

  const T &at(std::size_t idx) const {
    if (idx >= _length) {
      throw aether_bounds_exception_c(idx, _length);
    }
    return (*this)[idx];
  }

  std::size_t length() const noexcept { return _length; }
};

} // namespace truk::aether