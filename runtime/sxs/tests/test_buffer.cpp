#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

extern "C" {
#include <string.h>
#include <sxs/ds/buffer.h>
}

static int increment_byte(uint8_t *byte, size_t idx, void *callback_data) {
  (*byte)++;
  return 1;
}

static int stop_at_5(uint8_t *byte, size_t idx, void *callback_data) {
  if (idx >= 5) {
    return 0;
  }
  (*byte) *= 2;
  return 1;
}

static int skip_every_other(uint8_t *byte, size_t idx, void *callback_data) {
  (*byte) += 10;
  return 2;
}

static int count_callback_calls = 0;
static int counting_callback(uint8_t *byte, size_t idx, void *callback_data) {
  count_callback_calls++;
  return 1;
}

TEST_GROUP(BufferBasic){};

TEST(BufferBasic, CreateDestroy) {
  slp_buffer_t *buffer = slp_buffer_new(100);
  CHECK(buffer != NULL);
  CHECK(buffer->data != NULL);
  CHECK_EQUAL(100, buffer->capacity);
  CHECK_EQUAL(0, buffer->count);
  slp_buffer_free(buffer);
}

TEST(BufferBasic, CreateMinSize) {
  slp_buffer_t *buffer = slp_buffer_new(1);
  CHECK(buffer != NULL);
  CHECK_EQUAL(16, buffer->capacity);
  slp_buffer_free(buffer);
}

TEST(BufferBasic, CopyToBasic) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};

  int result = slp_buffer_copy_to(buffer, data, 5);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(5, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK(buf_data != NULL);
  CHECK_EQUAL(1, buf_data[0]);
  CHECK_EQUAL(2, buf_data[1]);
  CHECK_EQUAL(3, buf_data[2]);
  CHECK_EQUAL(4, buf_data[3]);
  CHECK_EQUAL(5, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferBasic, CopyToMultiple) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data1[] = {1, 2, 3};
  uint8_t data2[] = {4, 5, 6};
  uint8_t data3[] = {7, 8, 9};

  slp_buffer_copy_to(buffer, data1, 3);
  slp_buffer_copy_to(buffer, data2, 3);
  slp_buffer_copy_to(buffer, data3, 3);

  CHECK_EQUAL(9, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  for (int i = 0; i < 9; i++) {
    CHECK_EQUAL(i + 1, buf_data[i]);
  }

  slp_buffer_free(buffer);
}

TEST(BufferBasic, Reallocation) {
  slp_buffer_t *buffer = slp_buffer_new(8);
  CHECK_EQUAL(16, buffer->capacity);

  uint8_t data[40];
  for (int i = 0; i < 40; i++) {
    data[i] = i;
  }

  int result = slp_buffer_copy_to(buffer, data, 40);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(40, slp_buffer_count(buffer));
  CHECK(buffer->capacity >= 40);

  uint8_t *buf_data = slp_buffer_data(buffer);
  for (int i = 0; i < 40; i++) {
    CHECK_EQUAL(i, buf_data[i]);
  }

  slp_buffer_free(buffer);
}

TEST(BufferBasic, Clear) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};

  slp_buffer_copy_to(buffer, data, 5);
  CHECK_EQUAL(5, slp_buffer_count(buffer));

  slp_buffer_clear(buffer);
  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_copy_to(buffer, data, 3);
  CHECK_EQUAL(3, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST(BufferBasic, Empty) {
  slp_buffer_t *buffer = slp_buffer_new(32);

  CHECK_EQUAL(0, slp_buffer_count(buffer));

  count_callback_calls = 0;
  slp_buffer_for_each(buffer, counting_callback, NULL);
  CHECK_EQUAL(0, count_callback_calls);

  slp_buffer_free(buffer);
}

TEST(BufferBasic, NullChecks) {
  slp_buffer_free(NULL);

  CHECK_EQUAL(0, slp_buffer_count(NULL));
  POINTERS_EQUAL(NULL, slp_buffer_data(NULL));

  slp_buffer_clear(NULL);

  slp_buffer_t *buffer = slp_buffer_new(32);
  CHECK_EQUAL(-1, slp_buffer_copy_to(buffer, NULL, 10));
  CHECK_EQUAL(-1, slp_buffer_copy_to(NULL, (uint8_t *)"test", 4));

  slp_buffer_for_each(NULL, increment_byte, NULL);
  slp_buffer_for_each(buffer, NULL, NULL);

  slp_buffer_free(buffer);
}

TEST(BufferBasic, LargeData) {
  slp_buffer_t *buffer = slp_buffer_new(16);

  uint8_t large_data[1000];
  for (int i = 0; i < 1000; i++) {
    large_data[i] = i % 256;
  }

  int result = slp_buffer_copy_to(buffer, large_data, 1000);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(1000, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  for (int i = 0; i < 1000; i++) {
    CHECK_EQUAL(i % 256, buf_data[i]);
  }

  slp_buffer_free(buffer);
}

TEST(BufferBasic, ZeroLengthCopy) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3};

  int result = slp_buffer_copy_to(buffer, data, 0);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST_GROUP(BufferIteration){};

TEST(BufferIteration, ForEachIncrement) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {0, 1, 2, 3, 4};

  slp_buffer_copy_to(buffer, data, 5);
  slp_buffer_for_each(buffer, increment_byte, NULL);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(1, buf_data[0]);
  CHECK_EQUAL(2, buf_data[1]);
  CHECK_EQUAL(3, buf_data[2]);
  CHECK_EQUAL(4, buf_data[3]);
  CHECK_EQUAL(5, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferIteration, ForEachStop) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

  slp_buffer_copy_to(buffer, data, 10);
  slp_buffer_for_each(buffer, stop_at_5, NULL);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(2, buf_data[0]);
  CHECK_EQUAL(2, buf_data[1]);
  CHECK_EQUAL(2, buf_data[2]);
  CHECK_EQUAL(2, buf_data[3]);
  CHECK_EQUAL(2, buf_data[4]);
  CHECK_EQUAL(1, buf_data[5]);
  CHECK_EQUAL(1, buf_data[6]);
  CHECK_EQUAL(1, buf_data[7]);
  CHECK_EQUAL(1, buf_data[8]);
  CHECK_EQUAL(1, buf_data[9]);

  slp_buffer_free(buffer);
}

TEST(BufferIteration, ForEachSkip) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {0, 0, 0, 0, 0, 0, 0, 0};

  slp_buffer_copy_to(buffer, data, 8);
  slp_buffer_for_each(buffer, skip_every_other, NULL);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(10, buf_data[0]);
  CHECK_EQUAL(0, buf_data[1]);
  CHECK_EQUAL(10, buf_data[2]);
  CHECK_EQUAL(0, buf_data[3]);
  CHECK_EQUAL(10, buf_data[4]);
  CHECK_EQUAL(0, buf_data[5]);
  CHECK_EQUAL(10, buf_data[6]);
  CHECK_EQUAL(0, buf_data[7]);

  slp_buffer_free(buffer);
}

TEST_GROUP(BufferShrink){};

TEST(BufferShrink, ShrinkToFit) {
  slp_buffer_t *buffer = slp_buffer_new(100);
  CHECK_EQUAL(100, buffer->capacity);

  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);
  CHECK_EQUAL(5, slp_buffer_count(buffer));
  CHECK_EQUAL(100, buffer->capacity);

  int result = slp_buffer_shrink_to_fit(buffer);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(5, buffer->capacity);
  CHECK_EQUAL(5, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  for (int i = 0; i < 5; i++) {
    CHECK_EQUAL(i + 1, buf_data[i]);
  }

  slp_buffer_free(buffer);
}

TEST(BufferShrink, ShrinkToFitEmpty) {
  slp_buffer_t *buffer = slp_buffer_new(100);

  int result = slp_buffer_shrink_to_fit(buffer);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(100, buffer->capacity);

  slp_buffer_free(buffer);
}

TEST(BufferShrink, ShrinkToFitAlreadyFit) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[32];
  for (int i = 0; i < 32; i++) {
    data[i] = i;
  }

  slp_buffer_copy_to(buffer, data, 32);
  CHECK_EQUAL(32, buffer->capacity);
  CHECK_EQUAL(32, slp_buffer_count(buffer));

  int result = slp_buffer_shrink_to_fit(buffer);
  CHECK_EQUAL(0, result);
  CHECK_EQUAL(32, buffer->capacity);

  slp_buffer_free(buffer);
}

TEST(BufferShrink, ShrinkToFitNull) {
  int result = slp_buffer_shrink_to_fit(NULL);
  CHECK_EQUAL(-1, result);
}

TEST_GROUP(BufferSubBuffer){};

TEST(BufferSubBuffer, Validation) {
  slp_buffer_t *buffer = slp_buffer_new(100);
  uint8_t data[100];
  for (int i = 0; i < 100; i++) {
    data[i] = i;
  }
  slp_buffer_copy_to(buffer, data, 100);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 20, 30, &bytes_copied);

  CHECK(sub != NULL);
  CHECK_EQUAL(30, bytes_copied);
  CHECK_EQUAL(30, slp_buffer_count(sub));

  uint8_t *sub_data = slp_buffer_data(sub);
  for (int i = 0; i < 30; i++) {
    CHECK_EQUAL(20 + i, sub_data[i]);
  }

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, BasicExtraction) {
  slp_buffer_t *buffer = slp_buffer_new(50);
  uint8_t data[50];
  for (int i = 0; i < 50; i++) {
    data[i] = i;
  }
  slp_buffer_copy_to(buffer, data, 50);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 10, 20, &bytes_copied);

  CHECK(sub != NULL);
  CHECK_EQUAL(20, bytes_copied);
  CHECK_EQUAL(20, slp_buffer_count(sub));

  uint8_t *sub_data = slp_buffer_data(sub);
  for (int i = 0; i < 20; i++) {
    CHECK_EQUAL(10 + i, sub_data[i]);
  }

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, OffsetAtStart) {
  slp_buffer_t *buffer = slp_buffer_new(30);
  uint8_t data[30];
  for (int i = 0; i < 30; i++) {
    data[i] = i + 100;
  }
  slp_buffer_copy_to(buffer, data, 30);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 0, 15, &bytes_copied);

  CHECK(sub != NULL);
  CHECK_EQUAL(15, bytes_copied);
  CHECK_EQUAL(15, slp_buffer_count(sub));

  uint8_t *sub_data = slp_buffer_data(sub);
  for (int i = 0; i < 15; i++) {
    CHECK_EQUAL(i + 100, sub_data[i]);
  }

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, OffsetAtEnd) {
  slp_buffer_t *buffer = slp_buffer_new(50);
  uint8_t data[50];
  for (int i = 0; i < 50; i++) {
    data[i] = i * 2;
  }
  slp_buffer_copy_to(buffer, data, 50);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 45, 5, &bytes_copied);

  CHECK(sub != NULL);
  CHECK_EQUAL(5, bytes_copied);
  CHECK_EQUAL(5, slp_buffer_count(sub));

  uint8_t *sub_data = slp_buffer_data(sub);
  for (int i = 0; i < 5; i++) {
    CHECK_EQUAL((45 + i) * 2, sub_data[i]);
  }

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, LengthExceedsAvailable) {
  slp_buffer_t *buffer = slp_buffer_new(40);
  uint8_t data[40];
  for (int i = 0; i < 40; i++) {
    data[i] = 255 - i;
  }
  slp_buffer_copy_to(buffer, data, 40);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 30, 100, &bytes_copied);

  CHECK(sub != NULL);
  CHECK_EQUAL(10, bytes_copied);
  CHECK_EQUAL(10, slp_buffer_count(sub));

  uint8_t *sub_data = slp_buffer_data(sub);
  for (int i = 0; i < 10; i++) {
    CHECK_EQUAL(255 - (30 + i), sub_data[i]);
  }

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, ZeroLength) {
  slp_buffer_t *buffer = slp_buffer_new(30);
  uint8_t data[30];
  for (int i = 0; i < 30; i++) {
    data[i] = i;
  }
  slp_buffer_copy_to(buffer, data, 30);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 10, 0, &bytes_copied);

  CHECK(sub != NULL);
  CHECK_EQUAL(0, bytes_copied);
  CHECK_EQUAL(0, slp_buffer_count(sub));

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, InvalidOffset) {
  slp_buffer_t *buffer = slp_buffer_new(30);
  uint8_t data[30];
  for (int i = 0; i < 30; i++) {
    data[i] = i;
  }
  slp_buffer_copy_to(buffer, data, 30);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 50, 10, &bytes_copied);

  POINTERS_EQUAL(NULL, sub);
  CHECK_EQUAL(0, bytes_copied);

  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, BytesCopiedNull) {
  slp_buffer_t *buffer = slp_buffer_new(30);
  uint8_t data[30];
  for (int i = 0; i < 30; i++) {
    data[i] = i + 10;
  }
  slp_buffer_copy_to(buffer, data, 30);

  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 5, 10, NULL);

  CHECK(sub != NULL);
  CHECK_EQUAL(10, slp_buffer_count(sub));

  uint8_t *sub_data = slp_buffer_data(sub);
  for (int i = 0; i < 10; i++) {
    CHECK_EQUAL(5 + i + 10, sub_data[i]);
  }

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST(BufferSubBuffer, NullBuffer) {
  int bytes_copied = 99;
  slp_buffer_t *sub = slp_buffer_sub_buffer(NULL, 0, 10, &bytes_copied);

  POINTERS_EQUAL(NULL, sub);
  CHECK_EQUAL(0, bytes_copied);
}

TEST(BufferSubBuffer, SingleByte) {
  slp_buffer_t *buffer = slp_buffer_new(20);
  uint8_t data[20];
  for (int i = 0; i < 20; i++) {
    data[i] = i + 200;
  }
  slp_buffer_copy_to(buffer, data, 20);

  int bytes_copied = 0;
  slp_buffer_t *sub = slp_buffer_sub_buffer(buffer, 10, 1, &bytes_copied);

  CHECK(sub != NULL);
  CHECK_EQUAL(1, bytes_copied);
  CHECK_EQUAL(1, slp_buffer_count(sub));

  uint8_t *sub_data = slp_buffer_data(sub);
  CHECK_EQUAL(210, sub_data[0]);

  slp_buffer_free(sub);
  slp_buffer_free(buffer);
}

TEST_GROUP(BufferManipulation){};

TEST(BufferManipulation, RotateLeftBasic) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_rotate_left(buffer, 2);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(3, buf_data[0]);
  CHECK_EQUAL(4, buf_data[1]);
  CHECK_EQUAL(5, buf_data[2]);
  CHECK_EQUAL(1, buf_data[3]);
  CHECK_EQUAL(2, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateLeftZero) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {10, 20, 30, 40};
  slp_buffer_copy_to(buffer, data, 4);

  slp_buffer_rotate_left(buffer, 0);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(10, buf_data[0]);
  CHECK_EQUAL(20, buf_data[1]);
  CHECK_EQUAL(30, buf_data[2]);
  CHECK_EQUAL(40, buf_data[3]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateLeftExactCount) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_rotate_left(buffer, 5);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(1, buf_data[0]);
  CHECK_EQUAL(2, buf_data[1]);
  CHECK_EQUAL(3, buf_data[2]);
  CHECK_EQUAL(4, buf_data[3]);
  CHECK_EQUAL(5, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateLeftWrapping) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_rotate_left(buffer, 7);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(3, buf_data[0]);
  CHECK_EQUAL(4, buf_data[1]);
  CHECK_EQUAL(5, buf_data[2]);
  CHECK_EQUAL(1, buf_data[3]);
  CHECK_EQUAL(2, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateLeftEmpty) {
  slp_buffer_t *buffer = slp_buffer_new(32);

  slp_buffer_rotate_left(buffer, 5);

  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateLeftNull) { slp_buffer_rotate_left(NULL, 5); }

TEST(BufferManipulation, RotateRightBasic) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_rotate_right(buffer, 2);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(4, buf_data[0]);
  CHECK_EQUAL(5, buf_data[1]);
  CHECK_EQUAL(1, buf_data[2]);
  CHECK_EQUAL(2, buf_data[3]);
  CHECK_EQUAL(3, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateRightZero) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {10, 20, 30, 40};
  slp_buffer_copy_to(buffer, data, 4);

  slp_buffer_rotate_right(buffer, 0);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(10, buf_data[0]);
  CHECK_EQUAL(20, buf_data[1]);
  CHECK_EQUAL(30, buf_data[2]);
  CHECK_EQUAL(40, buf_data[3]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateRightExactCount) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_rotate_right(buffer, 5);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(1, buf_data[0]);
  CHECK_EQUAL(2, buf_data[1]);
  CHECK_EQUAL(3, buf_data[2]);
  CHECK_EQUAL(4, buf_data[3]);
  CHECK_EQUAL(5, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateRightWrapping) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_rotate_right(buffer, 7);

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(4, buf_data[0]);
  CHECK_EQUAL(5, buf_data[1]);
  CHECK_EQUAL(1, buf_data[2]);
  CHECK_EQUAL(2, buf_data[3]);
  CHECK_EQUAL(3, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateRightEmpty) {
  slp_buffer_t *buffer = slp_buffer_new(32);

  slp_buffer_rotate_right(buffer, 5);

  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST(BufferManipulation, RotateRightNull) { slp_buffer_rotate_right(NULL, 5); }

TEST(BufferManipulation, RotateLeftThenRight) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8};
  slp_buffer_copy_to(buffer, data, 8);

  slp_buffer_rotate_left(buffer, 3);
  slp_buffer_rotate_right(buffer, 3);

  uint8_t *buf_data = slp_buffer_data(buffer);
  for (int i = 0; i < 8; i++) {
    CHECK_EQUAL(i + 1, buf_data[i]);
  }

  slp_buffer_free(buffer);
}

TEST_GROUP(BufferTrim){};

TEST(BufferTrim, TrimLeftBasic) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {0, 0, 0, 5, 6, 0};
  slp_buffer_copy_to(buffer, data, 6);

  int result = slp_buffer_trim_left(buffer, 0);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(3, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(5, buf_data[0]);
  CHECK_EQUAL(6, buf_data[1]);
  CHECK_EQUAL(0, buf_data[2]);

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimLeftNoMatch) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  int result = slp_buffer_trim_left(buffer, 0);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(5, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(1, buf_data[0]);
  CHECK_EQUAL(2, buf_data[1]);
  CHECK_EQUAL(3, buf_data[2]);
  CHECK_EQUAL(4, buf_data[3]);
  CHECK_EQUAL(5, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimLeftAllMatch) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {7, 7, 7, 7, 7};
  slp_buffer_copy_to(buffer, data, 5);

  int result = slp_buffer_trim_left(buffer, 7);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimLeftPartialMatch) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {255, 255, 100, 255, 255};
  slp_buffer_copy_to(buffer, data, 5);

  int result = slp_buffer_trim_left(buffer, 255);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(3, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(100, buf_data[0]);
  CHECK_EQUAL(255, buf_data[1]);
  CHECK_EQUAL(255, buf_data[2]);

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimLeftEmpty) {
  slp_buffer_t *buffer = slp_buffer_new(32);

  int result = slp_buffer_trim_left(buffer, 0);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimLeftNull) {
  int result = slp_buffer_trim_left(NULL, 0);
  CHECK_EQUAL(-1, result);
}

TEST(BufferTrim, TrimRightBasic) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {0, 5, 6, 0, 0, 0};
  slp_buffer_copy_to(buffer, data, 6);

  int result = slp_buffer_trim_right(buffer, 0);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(3, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(0, buf_data[0]);
  CHECK_EQUAL(5, buf_data[1]);
  CHECK_EQUAL(6, buf_data[2]);

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimRightNoMatch) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  int result = slp_buffer_trim_right(buffer, 0);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(5, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(1, buf_data[0]);
  CHECK_EQUAL(2, buf_data[1]);
  CHECK_EQUAL(3, buf_data[2]);
  CHECK_EQUAL(4, buf_data[3]);
  CHECK_EQUAL(5, buf_data[4]);

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimRightAllMatch) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {7, 7, 7, 7, 7};
  slp_buffer_copy_to(buffer, data, 5);

  int result = slp_buffer_trim_right(buffer, 7);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimRightPartialMatch) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {255, 255, 100, 255, 255};
  slp_buffer_copy_to(buffer, data, 5);

  int result = slp_buffer_trim_right(buffer, 255);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(3, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(255, buf_data[0]);
  CHECK_EQUAL(255, buf_data[1]);
  CHECK_EQUAL(100, buf_data[2]);

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimRightEmpty) {
  slp_buffer_t *buffer = slp_buffer_new(32);

  int result = slp_buffer_trim_right(buffer, 0);

  CHECK_EQUAL(0, result);
  CHECK_EQUAL(0, slp_buffer_count(buffer));

  slp_buffer_free(buffer);
}

TEST(BufferTrim, TrimRightNull) {
  int result = slp_buffer_trim_right(NULL, 0);
  CHECK_EQUAL(-1, result);
}

TEST(BufferTrim, TrimBothSides) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {0, 0, 5, 6, 7, 0, 0};
  slp_buffer_copy_to(buffer, data, 7);

  slp_buffer_trim_left(buffer, 0);
  slp_buffer_trim_right(buffer, 0);

  CHECK_EQUAL(3, slp_buffer_count(buffer));

  uint8_t *buf_data = slp_buffer_data(buffer);
  CHECK_EQUAL(5, buf_data[0]);
  CHECK_EQUAL(6, buf_data[1]);
  CHECK_EQUAL(7, buf_data[2]);

  slp_buffer_free(buffer);
}

TEST_GROUP(BufferCopy){};

TEST(BufferCopy, CopyBufferBasic) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_t *copy = slp_buffer_copy(buffer);

  CHECK(copy != NULL);
  CHECK_EQUAL(5, slp_buffer_count(copy));
  CHECK_EQUAL(buffer->capacity, copy->capacity);

  uint8_t *copy_data = slp_buffer_data(copy);
  CHECK_EQUAL(1, copy_data[0]);
  CHECK_EQUAL(2, copy_data[1]);
  CHECK_EQUAL(3, copy_data[2]);
  CHECK_EQUAL(4, copy_data[3]);
  CHECK_EQUAL(5, copy_data[4]);

  slp_buffer_free(copy);
  slp_buffer_free(buffer);
}

TEST(BufferCopy, CopyBufferIndependence) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {10, 20, 30, 40, 50};
  slp_buffer_copy_to(buffer, data, 5);

  slp_buffer_t *copy = slp_buffer_copy(buffer);

  uint8_t *buf_data = slp_buffer_data(buffer);
  buf_data[0] = 99;
  buf_data[1] = 88;

  uint8_t *copy_data = slp_buffer_data(copy);
  CHECK_EQUAL(10, copy_data[0]);
  CHECK_EQUAL(20, copy_data[1]);
  CHECK_EQUAL(30, copy_data[2]);
  CHECK_EQUAL(40, copy_data[3]);
  CHECK_EQUAL(50, copy_data[4]);

  slp_buffer_free(copy);
  slp_buffer_free(buffer);
}

TEST(BufferCopy, CopyBufferEmpty) {
  slp_buffer_t *buffer = slp_buffer_new(32);

  slp_buffer_t *copy = slp_buffer_copy(buffer);

  CHECK(copy != NULL);
  CHECK_EQUAL(0, slp_buffer_count(copy));

  slp_buffer_free(copy);
  slp_buffer_free(buffer);
}

TEST(BufferCopy, CopyBufferNull) {
  slp_buffer_t *copy = slp_buffer_copy(NULL);
  POINTERS_EQUAL(NULL, copy);
}

TEST(BufferCopy, CopyBufferLarge) {
  slp_buffer_t *buffer = slp_buffer_new(1000);
  uint8_t data[1000];
  for (int i = 0; i < 1000; i++) {
    data[i] = i % 256;
  }
  slp_buffer_copy_to(buffer, data, 1000);

  slp_buffer_t *copy = slp_buffer_copy(buffer);

  CHECK(copy != NULL);
  CHECK_EQUAL(1000, slp_buffer_count(copy));

  uint8_t *copy_data = slp_buffer_data(copy);
  for (int i = 0; i < 1000; i++) {
    CHECK_EQUAL(i % 256, copy_data[i]);
  }

  slp_buffer_free(copy);
  slp_buffer_free(buffer);
}

TEST_GROUP(BufferSplit){};

TEST(BufferSplit, OutOfBoundsIndex) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  split_buffer_t split = slp_buffer_split(buffer, 10, 32, 32);

  POINTERS_EQUAL(NULL, split.left);
  POINTERS_EQUAL(NULL, split.right);

  slp_buffer_free(buffer);
}

TEST(BufferSplit, NullBuffer) {
  split_buffer_t split = slp_buffer_split(NULL, 2, 32, 32);

  POINTERS_EQUAL(NULL, split.left);
  POINTERS_EQUAL(NULL, split.right);
}

TEST(BufferSplit, EmptyBuffer) {
  slp_buffer_t *buffer = slp_buffer_new(32);

  split_buffer_t split = slp_buffer_split(buffer, 0, 32, 32);

  CHECK(split.left != NULL);
  CHECK(split.right != NULL);
  CHECK_EQUAL(0, slp_buffer_count(split.left));
  CHECK_EQUAL(0, slp_buffer_count(split.right));

  slp_split_buffer_free(&split);
  slp_buffer_free(buffer);
}

TEST(BufferSplit, ValueValidationBasic) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {10, 20, 30, 40, 50, 60, 70, 80};
  slp_buffer_copy_to(buffer, data, 8);

  split_buffer_t split = slp_buffer_split(buffer, 5, 32, 32);

  CHECK(split.left != NULL);
  CHECK(split.right != NULL);

  CHECK_EQUAL(5, slp_buffer_count(split.left));
  CHECK_EQUAL(3, slp_buffer_count(split.right));

  uint8_t *left_data = slp_buffer_data(split.left);
  CHECK_EQUAL(10, left_data[0]);
  CHECK_EQUAL(20, left_data[1]);
  CHECK_EQUAL(30, left_data[2]);
  CHECK_EQUAL(40, left_data[3]);
  CHECK_EQUAL(50, left_data[4]);

  uint8_t *right_data = slp_buffer_data(split.right);
  CHECK_EQUAL(60, right_data[0]);
  CHECK_EQUAL(70, right_data[1]);
  CHECK_EQUAL(80, right_data[2]);

  slp_split_buffer_free(&split);
  slp_buffer_free(buffer);
}

TEST(BufferSplit, IndexExclusive) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  slp_buffer_copy_to(buffer, data, 10);

  split_buffer_t split = slp_buffer_split(buffer, 3, 32, 32);

  CHECK(split.left != NULL);
  CHECK(split.right != NULL);

  CHECK_EQUAL(3, slp_buffer_count(split.left));
  CHECK_EQUAL(7, slp_buffer_count(split.right));

  uint8_t *left_data = slp_buffer_data(split.left);
  CHECK_EQUAL(0, left_data[0]);
  CHECK_EQUAL(1, left_data[1]);
  CHECK_EQUAL(2, left_data[2]);

  uint8_t *right_data = slp_buffer_data(split.right);
  CHECK_EQUAL(3, right_data[0]);
  CHECK_EQUAL(4, right_data[1]);
  CHECK_EQUAL(5, right_data[2]);

  slp_split_buffer_free(&split);
  slp_buffer_free(buffer);
}

TEST(BufferSplit, AtStart) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  split_buffer_t split = slp_buffer_split(buffer, 0, 32, 32);

  CHECK(split.left != NULL);
  CHECK(split.right != NULL);

  CHECK_EQUAL(0, slp_buffer_count(split.left));
  CHECK_EQUAL(5, slp_buffer_count(split.right));

  uint8_t *right_data = slp_buffer_data(split.right);
  for (int i = 0; i < 5; i++) {
    CHECK_EQUAL(i + 1, right_data[i]);
  }

  slp_split_buffer_free(&split);
  slp_buffer_free(buffer);
}

TEST(BufferSplit, AtEnd) {
  slp_buffer_t *buffer = slp_buffer_new(32);
  uint8_t data[] = {1, 2, 3, 4, 5};
  slp_buffer_copy_to(buffer, data, 5);

  split_buffer_t split = slp_buffer_split(buffer, 5, 32, 32);

  CHECK(split.left != NULL);
  CHECK(split.right != NULL);

  CHECK_EQUAL(5, slp_buffer_count(split.left));
  CHECK_EQUAL(0, slp_buffer_count(split.right));

  uint8_t *left_data = slp_buffer_data(split.left);
  for (int i = 0; i < 5; i++) {
    CHECK_EQUAL(i + 1, left_data[i]);
  }

  slp_split_buffer_free(&split);
  slp_buffer_free(buffer);
}

TEST(BufferSplit, DestroyNull) { slp_split_buffer_free(NULL); }

TEST(BufferSplit, DestroyPartial) {
  split_buffer_t split;
  split.left = slp_buffer_new(32);
  split.right = NULL;

  slp_split_buffer_free(&split);

  POINTERS_EQUAL(NULL, split.left);
  POINTERS_EQUAL(NULL, split.right);
}

int main(int argc, char **argv) {
  return CommandLineTestRunner::RunAllTests(argc, argv);
}
