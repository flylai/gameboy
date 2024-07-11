#include <gtest/gtest.h>

#include <fstream>

class StdoutSuppressor : public ::testing::EmptyTestEventListener {
  std::streambuf* original_buffer;
  std::ofstream null_stream;

  void OnTestStart(const ::testing::TestInfo&) override {
    original_buffer = std::cout.rdbuf();
#ifdef _WIN32
    null_stream.open("NUL");
#else
    null_stream.open("/dev/null");
#endif
    std::cout.rdbuf(null_stream.rdbuf());
  }

  void OnTestEnd(const ::testing::TestInfo&) override {
    std::cout.rdbuf(original_buffer);
    null_stream.close();
  }
};

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);

  ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
  listeners.Append(new StdoutSuppressor);

  return RUN_ALL_TESTS();
}
