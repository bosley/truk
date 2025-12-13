#include "truk/aether/forms.hpp"
#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(BasicMonadTests){
    void setup() override{}
    void teardown() override{}
};

TEST(BasicMonadTests, CanConstructI8) {
    truk::aether::i8_c val(42);
    CHECK_EQUAL(42, val.value());
}

TEST(BasicMonadTests, CanConstructI16) {
    truk::aether::i16_c val(1000);
    CHECK_EQUAL(1000, val.value());
}

TEST(BasicMonadTests, CanConstructI32) {
    truk::aether::i32_c val(100000);
    CHECK_EQUAL(100000, val.value());
}

TEST(BasicMonadTests, CanConstructI64) {
    truk::aether::i64_c val(10000000000ULL);
    CHECK_EQUAL(10000000000ULL, val.value());
}

TEST(BasicMonadTests, CanConstructR32) {
    truk::aether::r32_c val(3.14f);
    DOUBLES_EQUAL(3.14f, val.value(), 0.001);
}

TEST(BasicMonadTests, CanConstructR64) {
    truk::aether::r64_c val(3.14159265359);
    DOUBLES_EQUAL(3.14159265359, val.value(), 0.0000001);
}

TEST(BasicMonadTests, I8Arithmetic) {
    truk::aether::i8_c a(10);
    truk::aether::i8_c b(5);
    
    auto sum = a + b;
    CHECK_EQUAL(15, sum.value());
    
    auto diff = a - b;
    CHECK_EQUAL(5, diff.value());
    
    auto prod = a * b;
    CHECK_EQUAL(50, prod.value());
    
    auto quot = a / b;
    CHECK_EQUAL(2, quot.value());
}

TEST_GROUP(DynamicMonadTests){
    void setup() override{}
    void teardown() override{}
};

TEST(DynamicMonadTests, CanConstructEmpty) {
    truk::aether::dynamic_t dyn;
    CHECK_EQUAL(0, dyn.bytes.size());
}

TEST(DynamicMonadTests, CanConstructWithVector) {
    std::vector<std::uint8_t> data = {1, 2, 3, 4, 5};
    truk::aether::dynamic_t dyn(data);
    CHECK_EQUAL(5, dyn.bytes.size());
    CHECK_EQUAL(1, dyn.bytes[0]);
    CHECK_EQUAL(5, dyn.bytes[4]);
}

TEST(DynamicMonadTests, CanConstructWithPointer) {
    std::uint8_t data[] = {10, 20, 30};
    truk::aether::dynamic_t dyn(data, 3);
    CHECK_EQUAL(3, dyn.bytes.size());
    CHECK_EQUAL(10, dyn.bytes[0]);
    CHECK_EQUAL(30, dyn.bytes[2]);
}

TEST_GROUP(BytePackingTests){
    void setup() override{}
    void teardown() override{}
};

TEST(BytePackingTests, PackI8LittleEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint8_t>(0x42, true);
    CHECK_EQUAL(1, dyn.size());
    CHECK_EQUAL(0x42, dyn.bytes[0]);
}

TEST(BytePackingTests, PackI16LittleEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint16_t>(0x1234, true);
    CHECK_EQUAL(2, dyn.size());
    CHECK_EQUAL(0x34, dyn.bytes[0]);
    CHECK_EQUAL(0x12, dyn.bytes[1]);
}

TEST(BytePackingTests, PackI32LittleEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0x12345678, true);
    CHECK_EQUAL(4, dyn.size());
    CHECK_EQUAL(0x78, dyn.bytes[0]);
    CHECK_EQUAL(0x56, dyn.bytes[1]);
    CHECK_EQUAL(0x34, dyn.bytes[2]);
    CHECK_EQUAL(0x12, dyn.bytes[3]);
}

TEST(BytePackingTests, PackI64LittleEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint64_t>(0x123456789ABCDEF0ULL, true);
    CHECK_EQUAL(8, dyn.size());
    CHECK_EQUAL(0xF0, dyn.bytes[0]);
    CHECK_EQUAL(0xDE, dyn.bytes[1]);
    CHECK_EQUAL(0xBC, dyn.bytes[2]);
    CHECK_EQUAL(0x9A, dyn.bytes[3]);
}

TEST(BytePackingTests, PackI16BigEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint16_t>(0x1234, false);
    CHECK_EQUAL(2, dyn.size());
    CHECK_EQUAL(0x12, dyn.bytes[0]);
    CHECK_EQUAL(0x34, dyn.bytes[1]);
}

TEST(BytePackingTests, PackI32BigEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0x12345678, false);
    CHECK_EQUAL(4, dyn.size());
    CHECK_EQUAL(0x12, dyn.bytes[0]);
    CHECK_EQUAL(0x34, dyn.bytes[1]);
    CHECK_EQUAL(0x56, dyn.bytes[2]);
    CHECK_EQUAL(0x78, dyn.bytes[3]);
}

TEST(BytePackingTests, PackMultipleValues) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint8_t>(0x11, true);
    dyn.pack<std::uint16_t>(0x2233, true);
    dyn.pack<std::uint32_t>(0x44556677, true);
    CHECK_EQUAL(7, dyn.size());
    CHECK_EQUAL(0x11, dyn.bytes[0]);
    CHECK_EQUAL(0x33, dyn.bytes[1]);
    CHECK_EQUAL(0x22, dyn.bytes[2]);
}

TEST(BytePackingTests, AppendByte) {
    truk::aether::dynamic_t dyn;
    dyn.append_byte(0xAA);
    dyn.append_byte(0xBB);
    CHECK_EQUAL(2, dyn.size());
    CHECK_EQUAL(0xAA, dyn.bytes[0]);
    CHECK_EQUAL(0xBB, dyn.bytes[1]);
}

TEST(BytePackingTests, AppendBytes) {
    truk::aether::dynamic_t dyn;
    std::uint8_t data[] = {0x10, 0x20, 0x30};
    dyn.append_bytes(data, 3);
    CHECK_EQUAL(3, dyn.size());
    CHECK_EQUAL(0x10, dyn.bytes[0]);
    CHECK_EQUAL(0x20, dyn.bytes[1]);
    CHECK_EQUAL(0x30, dyn.bytes[2]);
}

TEST(BytePackingTests, UnpackI8) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint8_t>(0x42, true);
    auto result = dyn.unpack<std::uint8_t>(0, true);
    CHECK_EQUAL(0x42, result);
}

TEST(BytePackingTests, UnpackI16LittleEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint16_t>(0x1234, true);
    auto result = dyn.unpack<std::uint16_t>(0, true);
    CHECK_EQUAL(0x1234, result);
}

TEST(BytePackingTests, UnpackI32LittleEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0x12345678, true);
    auto result = dyn.unpack<std::uint32_t>(0, true);
    CHECK_EQUAL(0x12345678, result);
}

TEST(BytePackingTests, UnpackI64LittleEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint64_t>(0x123456789ABCDEF0ULL, true);
    auto result = dyn.unpack<std::uint64_t>(0, true);
    CHECK_EQUAL(0x123456789ABCDEF0ULL, result);
}

TEST(BytePackingTests, UnpackI16BigEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint16_t>(0x1234, false);
    auto result = dyn.unpack<std::uint16_t>(0, false);
    CHECK_EQUAL(0x1234, result);
}

TEST(BytePackingTests, UnpackI32BigEndian) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0x12345678, false);
    auto result = dyn.unpack<std::uint32_t>(0, false);
    CHECK_EQUAL(0x12345678, result);
}

TEST(BytePackingTests, RoundTripFloat) {
    truk::aether::dynamic_t dyn;
    float original = 3.14159f;
    dyn.pack<float>(original, true);
    auto result = dyn.unpack<float>(0, true);
    DOUBLES_EQUAL(original, result, 0.00001);
}

TEST(BytePackingTests, RoundTripDouble) {
    truk::aether::dynamic_t dyn;
    double original = 3.14159265359;
    dyn.pack<double>(original, true);
    auto result = dyn.unpack<double>(0, true);
    DOUBLES_EQUAL(original, result, 0.0000000001);
}

TEST(BytePackingTests, UnpackMultipleValues) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint8_t>(0x11, true);
    dyn.pack<std::uint16_t>(0x2233, true);
    dyn.pack<std::uint32_t>(0x44556677, true);
    
    auto val1 = dyn.unpack<std::uint8_t>(0, true);
    auto val2 = dyn.unpack<std::uint16_t>(1, true);
    auto val3 = dyn.unpack<std::uint32_t>(3, true);
    
    CHECK_EQUAL(0x11, val1);
    CHECK_EQUAL(0x2233, val2);
    CHECK_EQUAL(0x44556677, val3);
}

TEST(BytePackingTests, UnpackWithOffset) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0xAAAAAAAA, true);
    dyn.pack<std::uint32_t>(0xBBBBBBBB, true);
    dyn.pack<std::uint32_t>(0xCCCCCCCC, true);
    
    auto val1 = dyn.unpack<std::uint32_t>(0, true);
    auto val2 = dyn.unpack<std::uint32_t>(4, true);
    auto val3 = dyn.unpack<std::uint32_t>(8, true);
    
    CHECK_EQUAL(0xAAAAAAAA, val1);
    CHECK_EQUAL(0xBBBBBBBB, val2);
    CHECK_EQUAL(0xCCCCCCCC, val3);
}

TEST(BytePackingTests, UnpackOutOfBoundsReturnsZero) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint16_t>(0x1234, true);
    
    auto result = dyn.unpack<std::uint32_t>(0, true);
    CHECK_EQUAL(0, result);
}

TEST(BytePackingTests, UnpackOffsetTooLargeReturnsZero) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0x12345678, true);
    
    auto result = dyn.unpack<std::uint32_t>(10, true);
    CHECK_EQUAL(0, result);
}

TEST(BytePackingTests, EmptyDynamicHasZeroSize) {
    truk::aether::dynamic_t dyn;
    CHECK_EQUAL(0, dyn.size());
}

TEST(BytePackingTests, ClearResetsDynamic) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0x12345678, true);
    CHECK_EQUAL(4, dyn.size());
    
    dyn.clear();
    CHECK_EQUAL(0, dyn.size());
}

TEST(BytePackingTests, AtAccessesByte) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint16_t>(0x1234, true);
    
    CHECK_EQUAL(0x34, dyn.at(0));
    CHECK_EQUAL(0x12, dyn.at(1));
}

TEST(BytePackingTests, DynamicBaseIfPackValue) {
    truk::aether::dynamic_t dyn;
    truk::aether::dynamic_base_if base(std::move(dyn));
    
    base.pack_value<std::uint32_t>(0xDEADBEEF, true);
    CHECK_EQUAL(4, base.byte_size());
}

TEST(BytePackingTests, DynamicBaseIfUnpackValue) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint32_t>(0xCAFEBABE, true);
    truk::aether::dynamic_base_if base(std::move(dyn));
    
    auto result = base.unpack_value<std::uint32_t>(0, true);
    CHECK_EQUAL(0xCAFEBABE, result);
}

TEST(BytePackingTests, DynamicBaseIfGetBytes) {
    truk::aether::dynamic_t dyn;
    dyn.pack<std::uint16_t>(0xABCD, true);
    truk::aether::dynamic_base_if base(std::move(dyn));
    
    const auto& bytes = base.get_bytes();
    CHECK_EQUAL(2, bytes.size());
    CHECK_EQUAL(0xCD, bytes[0]);
    CHECK_EQUAL(0xAB, bytes[1]);
}

TEST(BytePackingTests, DynamicBaseIfByteSize) {
    truk::aether::dynamic_t dyn;
    truk::aether::dynamic_base_if base(std::move(dyn));
    
    CHECK_EQUAL(0, base.byte_size());
    
    base.pack_value<std::uint8_t>(0xFF, true);
    CHECK_EQUAL(1, base.byte_size());
    
    base.pack_value<std::uint32_t>(0x12345678, true);
    CHECK_EQUAL(5, base.byte_size());
}

int main(int argc, char **argv) {
    return CommandLineTestRunner::RunAllTests(argc, argv);
}
