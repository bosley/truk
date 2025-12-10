#pragma once
#include <array>
#include <cstddef>
#include <string>

namespace truk::core {

static constexpr std::size_t INTEGER_REGISTER_COUNT = 12;
static constexpr std::size_t REAL_REGISTER_COUNT = 4;

enum class INT_REG {
  ZERO = 0,
  ONE,
  TWO,
  THREE,
  FOUR,
  FIVE,
  SIX,
  SEVEN,
  EIGHT,
  NINE,
  TEN,
  ELEVEN
};

enum class REAL_REG {
  ZERO = 0,
  ONE,
  TWO,
  THREE
};

class processor_c {
public:
  // Exception type for processor errors
  class processor_exception_c : public std::exception {
  public:
    explicit processor_exception_c(const char *msg) noexcept : _msg(msg) {}

    explicit processor_exception_c(const std::string &msg) noexcept
        : _msg(msg) {}

    const char *what() const noexcept override { return _msg.c_str(); }

  private:
    std::string _msg;
  };

  class alu_if {
  public:
    virtual ~alu_if() = default;

    // Integer operations
    virtual void i_add(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;
    virtual void i_sub(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;
    virtual void i_mul(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;
    virtual void i_div(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;
    virtual void i_mod(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;

    // bitwise unary and binary operations on integers
    virtual void i_lsh(INT_REG target, INT_REG amount) = 0;
    virtual void i_rsh(INT_REG target, INT_REG amount) = 0;
    virtual void i_and(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;
    virtual void i_or(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;
    virtual void i_xor(INT_REG dest, INT_REG lhs, INT_REG rhs) = 0;
    virtual void i_not(INT_REG dest, INT_REG src) = 0;

    // Floating-point operations
    virtual void f_add(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) = 0;
    virtual void f_sub(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) = 0;
    virtual void f_mul(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) = 0;
    virtual void f_div(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) = 0;
  };

  class reg_if {
  public:
    virtual ~reg_if() = default;

    virtual std::int64_t get_int(INT_REG reg) const = 0;
    virtual void set_int(INT_REG reg, std::int64_t value) = 0;

    virtual double get_real(REAL_REG reg) const = 0;
    virtual void set_real(REAL_REG reg, double value) = 0;
  };

private:
  class alu_impl_c : public alu_if {
  public:
    alu_impl_c() = delete;
    alu_impl_c(processor_c &proc) : _proc(proc) {}

    // Integer operations
    void i_add(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] +
          _proc._int_registers[static_cast<std::size_t>(rhs)];
    }
    void i_sub(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] -
          _proc._int_registers[static_cast<std::size_t>(rhs)];
    }
    void i_mul(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] *
          _proc._int_registers[static_cast<std::size_t>(rhs)];
    }
    void i_div(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      auto divisor = _proc._int_registers[static_cast<std::size_t>(rhs)];
      if (divisor == 0)
        throw processor_exception_c("Division by zero");
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] / divisor;
    }
    void i_mod(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      auto divisor = _proc._int_registers[static_cast<std::size_t>(rhs)];
      if (divisor == 0)
        throw processor_exception_c("Modulo by zero");
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] % divisor;
    }

    // bitwise unary and binary operations on integers
    void i_lsh(INT_REG target, INT_REG amount) override {
      _proc._int_registers[static_cast<std::size_t>(target)] <<=
          static_cast<std::size_t>(
              _proc._int_registers[static_cast<std::size_t>(amount)]);
    }
    void i_rsh(INT_REG target, INT_REG amount) override {
      _proc._int_registers[static_cast<std::size_t>(target)] >>=
          static_cast<std::size_t>(
              _proc._int_registers[static_cast<std::size_t>(amount)]);
    }
    void i_and(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] &
          _proc._int_registers[static_cast<std::size_t>(rhs)];
    }
    void i_or(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] |
          _proc._int_registers[static_cast<std::size_t>(rhs)];
    }
    void i_xor(INT_REG dest, INT_REG lhs, INT_REG rhs) override {
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          _proc._int_registers[static_cast<std::size_t>(lhs)] ^
          _proc._int_registers[static_cast<std::size_t>(rhs)];
    }
    void i_not(INT_REG dest, INT_REG src) override {
      _proc._int_registers[static_cast<std::size_t>(dest)] =
          ~_proc._int_registers[static_cast<std::size_t>(src)];
    }

    // Floating-point operations
    void f_add(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) override {
      _proc._real_registers[static_cast<std::size_t>(dest)] =
          _proc._real_registers[static_cast<std::size_t>(lhs)] +
          _proc._real_registers[static_cast<std::size_t>(rhs)];
    }
    void f_sub(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) override {
      _proc._real_registers[static_cast<std::size_t>(dest)] =
          _proc._real_registers[static_cast<std::size_t>(lhs)] -
          _proc._real_registers[static_cast<std::size_t>(rhs)];
    }
    void f_mul(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) override {
      _proc._real_registers[static_cast<std::size_t>(dest)] =
          _proc._real_registers[static_cast<std::size_t>(lhs)] *
          _proc._real_registers[static_cast<std::size_t>(rhs)];
    }
    void f_div(REAL_REG dest, REAL_REG lhs, REAL_REG rhs) override {
      double divisor = _proc._real_registers[static_cast<std::size_t>(rhs)];
      if (divisor == 0.0)
        throw processor_exception_c("Floating-point division by zero");
      _proc._real_registers[static_cast<std::size_t>(dest)] =
          _proc._real_registers[static_cast<std::size_t>(lhs)] / divisor;
    }

  private:
    processor_c &_proc;
  };

  class reg_impl_c : public reg_if {
  public:
    reg_impl_c() = delete;
    reg_impl_c(processor_c &proc) : _proc(proc) {}

    std::int64_t get_int(INT_REG reg) const override {
      return _proc._int_registers[static_cast<std::size_t>(reg)];
    }

    void set_int(INT_REG reg, std::int64_t value) override {
      _proc._int_registers[static_cast<std::size_t>(reg)] = value;
    }

    double get_real(REAL_REG reg) const override {
      return _proc._real_registers[static_cast<std::size_t>(reg)];
    }

    void set_real(REAL_REG reg, double value) override {
      _proc._real_registers[static_cast<std::size_t>(reg)] = value;
    }

  private:
    processor_c &_proc;
  };

  std::array<std::int64_t, INTEGER_REGISTER_COUNT> _int_registers;
  std::array<double, REAL_REGISTER_COUNT> _real_registers;
};

} // namespace truk::core
