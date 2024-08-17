// circle_buffer_test.cpp
#include "common/circle_buffer.h"

#include <gtest/gtest.h>

#include "common/type.h"

namespace gb {

class CircleBufferTest : public ::testing::Test {
protected:
  CircleBuffer<i32> buffer_{5};
};

TEST_F(CircleBufferTest, PushIncreasesSize) {
  buffer_.push(1);
  EXPECT_EQ(buffer_.size(), 1);
}

TEST_F(CircleBufferTest, PopDecreasesSize) {
  buffer_.push(1);
  buffer_.pop();
  EXPECT_EQ(buffer_.size(), 0);
}

TEST_F(CircleBufferTest, PopReturnsCorrectValue) {
  buffer_.push(1);
  EXPECT_EQ(buffer_.pop(), 1);
}

TEST_F(CircleBufferTest, FrontReturnsCorrectValue) {
  buffer_.push(1);
  EXPECT_EQ(buffer_.front(), 1);
}

TEST_F(CircleBufferTest, BackReturnsCorrectValue) {
  buffer_.push(1);
  EXPECT_EQ(buffer_.back(), 1);
}

TEST_F(CircleBufferTest, FullBufferOverwritesOldestValue) {
  for (int i = 0; i < 6; ++i) {
    buffer_.push(i);
  }
  EXPECT_EQ(buffer_.front(), 1);
}

TEST_F(CircleBufferTest, PopNRemovesCorrectNumberOfElements) {
  for (int i = 0; i < 5; ++i) {
    buffer_.push(i);
  }
  buffer_.popN(3);
  EXPECT_EQ(buffer_.size(), 2);
  EXPECT_EQ(buffer_.front(), 3);
}

TEST_F(CircleBufferTest, ClearEmptiesBuffer) {
  buffer_.push(1);
  buffer_.clear();
  EXPECT_TRUE(buffer_.empty());
}

TEST_F(CircleBufferTest, EmptyBufferReturnsZeroOnPop) { EXPECT_EQ(buffer_.pop(), 0); }

TEST_F(CircleBufferTest, EmptyBufferReturnsZeroOnFront) { EXPECT_EQ(buffer_.front(), 0); }

TEST_F(CircleBufferTest, EmptyBufferReturnsZeroOnBack) { EXPECT_EQ(buffer_.back(), 0); }

TEST_F(CircleBufferTest, FullReturnsTrueWhenBufferIsFull) {
  for (int i = 0; i < 5; ++i) {
    buffer_.push(i);
  }
  EXPECT_TRUE(buffer_.full());
}

TEST_F(CircleBufferTest, PushToFullBufferOverwritesOldestValue) {
  for (int i = 0; i < 5; ++i) {
    buffer_.push(i);
  }
  buffer_.push(5);
  EXPECT_EQ(buffer_.front(), 1);
  EXPECT_EQ(buffer_.back(), 5);
}

TEST_F(CircleBufferTest, PopFromEmptyBufferReturnsZero) { EXPECT_EQ(buffer_.pop(), 0); }

TEST_F(CircleBufferTest, PopNFromEmptyBufferDoesNothing) {
  buffer_.popN(3);
  EXPECT_TRUE(buffer_.empty());
}

TEST_F(CircleBufferTest, PopNMoreThanSizeClearsBuffer) {
  for (int i = 0; i < 3; ++i) {
    buffer_.push(i);
  }
  buffer_.popN(5);
  EXPECT_TRUE(buffer_.empty());
}

TEST_F(CircleBufferTest, PushPopAlternating) {
  for (int i = 0; i < 10; ++i) {
    buffer_.push(i);
    EXPECT_EQ(buffer_.pop(), i);
  }
  EXPECT_TRUE(buffer_.empty());
}

TEST_F(CircleBufferTest, PushPopAlternatingWithOverflow) {
  for (int i = 0; i < 10; ++i) {
    if (i >= 5) {
      EXPECT_EQ(buffer_.pop(), i - 5);
    }
    buffer_.push(i);
  }
  EXPECT_EQ(buffer_.size(), 5);
  EXPECT_EQ(buffer_.front(), 5);
  EXPECT_EQ(buffer_.back(), 9);
}

TEST_F(CircleBufferTest, PushPopEmptyBuffer) {
  EXPECT_EQ(buffer_.pop(), 0);
  buffer_.push(1);
  EXPECT_EQ(buffer_.pop(), 1);
  EXPECT_TRUE(buffer_.empty());
}

TEST_F(CircleBufferTest, PushPopDifferentDataTypes) {
  CircleBuffer<double> doubleBuffer(3);
  doubleBuffer.push(1.1);
  doubleBuffer.push(2.2);
  doubleBuffer.push(3.3);
  EXPECT_EQ(doubleBuffer.pop(), 1.1);
  EXPECT_EQ(doubleBuffer.pop(), 2.2);
  EXPECT_EQ(doubleBuffer.pop(), 3.3);

  CircleBuffer<std::string> stringBuffer(2);
  stringBuffer.push("hello");
  stringBuffer.push("world");
  EXPECT_EQ(stringBuffer.pop(), "hello");
  EXPECT_EQ(stringBuffer.pop(), "world");
}

TEST_F(CircleBufferTest, BufferCapacityOne) {
  CircleBuffer<int> singleBuffer(1);
  singleBuffer.push(1);
  EXPECT_EQ(singleBuffer.size(), 1);
  EXPECT_EQ(singleBuffer.front(), 1);
  EXPECT_EQ(singleBuffer.back(), 1);

  singleBuffer.push(2);
  EXPECT_EQ(singleBuffer.size(), 1);
  EXPECT_EQ(singleBuffer.front(), 2);
  EXPECT_EQ(singleBuffer.back(), 2);

  EXPECT_EQ(singleBuffer.pop(), 2);
  EXPECT_TRUE(singleBuffer.empty());
}

TEST_F(CircleBufferTest, LargeDataSet) {
  const size_t largeSize = 1000000;
  CircleBuffer<int> largeBuffer(largeSize);
  for (size_t i = 0; i < largeSize; ++i) {
    largeBuffer.push(i);
  }
  EXPECT_EQ(largeBuffer.size(), largeSize);
  for (size_t i = 0; i < largeSize; ++i) {
    EXPECT_EQ(largeBuffer.pop(), i);
  }
  EXPECT_TRUE(largeBuffer.empty());
}

TEST_F(CircleBufferTest, BufferCapacityTwo) {
  CircleBuffer<int> buffer(2);
  buffer.push(1);
  buffer.push(2);
  EXPECT_EQ(buffer.size(), 2);
  EXPECT_EQ(buffer.front(), 1);
  EXPECT_EQ(buffer.back(), 2);

  buffer.push(3);
  EXPECT_EQ(buffer.size(), 2);
  EXPECT_EQ(buffer.front(), 2);
  EXPECT_EQ(buffer.back(), 3);

  EXPECT_EQ(buffer.pop(), 2);
  EXPECT_EQ(buffer.pop(), 3);
  EXPECT_TRUE(buffer.empty());
}

TEST_F(CircleBufferTest, BufferCapacityTen) {
  CircleBuffer<int> buffer(10);
  for (int i = 1; i <= 10; ++i) {
    buffer.push(i);
  }
  EXPECT_EQ(buffer.size(), 10);
  EXPECT_EQ(buffer.front(), 1);
  EXPECT_EQ(buffer.back(), 10);

  buffer.push(11);
  EXPECT_EQ(buffer.size(), 10);
  EXPECT_EQ(buffer.front(), 2);
  EXPECT_EQ(buffer.back(), 11);

  for (int i = 2; i <= 11; ++i) {
    EXPECT_EQ(buffer.pop(), i);
  }
  EXPECT_TRUE(buffer.empty());
}

TEST_F(CircleBufferTest, BufferCapacityHundred) {
  CircleBuffer<int> buffer(100);
  for (int i = 1; i <= 100; ++i) {
    buffer.push(i);
  }
  EXPECT_EQ(buffer.size(), 100);
  EXPECT_EQ(buffer.front(), 1);
  EXPECT_EQ(buffer.back(), 100);

  buffer.push(101);
  EXPECT_EQ(buffer.size(), 100);
  EXPECT_EQ(buffer.front(), 2);
  EXPECT_EQ(buffer.back(), 101);

  for (int i = 2; i <= 101; ++i) {
    EXPECT_EQ(buffer.pop(), i);
  }
  EXPECT_TRUE(buffer.empty());
}

} // namespace gb
