// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include <iostream>
#include <cassert>
#include <cstring>
#include "extractor/filter.h"

using namespace ext;

void TestCase(const char* key, std::string s, bool expect) {
  int type = MakeType(key);
  auto func = FilterFactory(type);
  assert(func);
  bool ret = func(&s);
  assert(ret == expect);
  std::cout << s << std::endl;
}

int main() {
  TestCase("APP_IMSI", "460087495038274", true);
  TestCase("APP_IMEI", "99008749503827", true);
  TestCase("APP_IMEI", "980087495038277", true);
  TestCase("APP_IMEI", "A2008749503827", true);
  TestCase("APP_LONGITUDE", "12.0001", true);
  TestCase("APP_LATITUDE", "190.0001", false);
  TestCase("APP_MAC", "a0:b1:c2:d3:e4:f5", true);
  TestCase("APP_MAC", "00-g0-00-00-00-00", false);
  TestCase("APP_MAC", "0A-0B-0C-0D-0E-0F", true);
  TestCase("EMAIL", "yuyue2200@hotmail.com", true);
  return 0;
}


