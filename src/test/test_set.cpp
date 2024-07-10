
#include <gtest/gtest.h>

#include <filesystem>

#include "test_base.h"

using namespace gb;

namespace fs          = std::filesystem;
constexpr u64 TIMEOUT = 60; // seconds

struct TestParams {
  std::string path;
  Monitor::CheckFunction result_checker;

  friend std::ostream &operator<<(std::ostream &o, const TestParams &param) { return o << param.path; }
};

class GBTest : public ::testing::TestWithParam<TestParams> {
public:
  static std::string ParamToString(const testing::TestParamInfo<TestParams> &info) {
    std::stringstream ss;
    const std::string &path = info.param.path;
    int pos                 = path.find_last_of('/');
    pos++; // skip '/' and if `/` not found, pos = 0
    while (pos < path.size()) {
      if (isalnum(path[pos])) {
        ss << path[pos];
      } else {
        ss << '_';
      }
      pos++;
    }
    return ss.str();
  }
};

static std::vector<TestParams> getFileList(const std::string &path,
                                           const Monitor::CheckFunction &result_checker, bool recursive) {
  std::vector<TestParams> v;
  auto checkFile = [&](const fs::directory_entry &entry) {
    if (entry.is_regular_file() && entry.path().extension() == ".gb") {
      std::string filename = entry.path();
      v.push_back({filename, result_checker});
    }
  };
  if (!recursive) {
    for (const auto &entry: fs::directory_iterator(path)) {
      checkFile(entry);
    }
  } else {
    for (const auto &entry: fs::recursive_directory_iterator(path)) {
      checkFile(entry);
    }
  }
  return v;
}

static Monitor::CheckStatus gb_test_roms_checker(const std::vector<u8> &v) {
  if (v.size() < 4) {
    return Monitor::kUNKNOWN;
  }
  std::string str{v.begin(), v.end()};
  if (str.find("Pass") != std::string::npos) {
    return Monitor::kSUCCESS;
  } else if (str.find("Failed") != std::string::npos) {
    return Monitor::kFAILED;
  } else {
    return Monitor::kUNKNOWN;
  }
}

static Monitor::CheckStatus mts_checker(const std::vector<u8> &v) {
  const std::string success{3, 5, 8, 13, 21, 34};
  const std::string fail{'B', 'B', 'B', 'B', 'B', 'B'};
  if (v.size() < success.size()) {
    return Monitor::kUNKNOWN;
  }
  std::string str{v.begin(), v.end()};
  if (str.find(success) != std::string::npos) {
    return Monitor::kSUCCESS;
  } else if (str.find(fail) != std::string::npos) {
    return Monitor::kFAILED;
  } else {
    return Monitor::kUNKNOWN;
  }
}

TEST_P(GBTest, ARGS) {
  TestParams param = GetParam();
  GameBoy gb(param.path);
  Monitor monitor(&gb, param.result_checker);
  EXPECT_TRUE(monitor.run(TIMEOUT));
}

INSTANTIATE_TEST_SUITE_P(gb_test_roms_cpu_instrs, GBTest,
                         ::testing::ValuesIn(getFileList("../tests/gb-test-roms/cpu_instrs/",
                                                         gb_test_roms_checker, true)),
                         GBTest::ParamToString);

INSTANTIATE_TEST_SUITE_P(gb_test_roms_instr_timing, GBTest,
                         ::testing::ValuesIn(getFileList("../tests/gb-test-roms/instr_timing",
                                                         gb_test_roms_checker, true)),
                         GBTest::ParamToString);

INSTANTIATE_TEST_SUITE_P(gb_test_roms_interrupt_time, GBTest,
                         ::testing::ValuesIn(getFileList("../tests/gb-test-roms/interrupt_time",
                                                         gb_test_roms_checker, false)),
                         GBTest::ParamToString);


INSTANTIATE_TEST_SUITE_P(mts, GBTest,
                         ::testing::ValuesIn(getFileList("../tests/mts/acceptance/timer/", mts_checker,
                                                         false)),
                         GBTest::ParamToString);
