// Author: yuyue/X3130 (yuyue2200@hotmail.com)
// File: rule_test.cc

#include <iostream>
#include <cstring>
#include <fstream>

#include "extractor/rule.h"
#include "extractor/extractor.h"
#include "rule_define.h"

void ReadFile(const char* fname, std::string* out) {
  char tmp[4096];
  size_t size = 0;
  std::ifstream in(fname);
  while ((size = in.readsome(tmp, 4096)) > 0)
    out->append(tmp, size);
}

void Extract(const char* http_file, const char* rule_file) {
  std::string buf;
  ReadFile(http_file, &buf);
  std::string rule;
  ReadFile(rule_file, &rule);

  ext::Extractor ext(rule.data(), rule.size());
  std::vector<ext::Record> res;
  ext::Record attrib;
  int ret = ext.Extract(buf.data(), buf.size(), &res, &attrib);
  if (ret != 0) {
    std::cout << ext.StringError(ret) << std::endl;
  }

  for (size_t i = 0; i < res.size(); ++i) {
    auto& records = res[i];
    for (auto iter = records.begin(); iter != records.end(); ++iter) {
      std::cout << " " << iter->first << ":" << iter->second;
    }
    std::cout << std::endl;
  }
}

int main(int argc, char* argv[]) {
  if (argc == 3) {
    Extract(argv[1], argv[2]);
  } else {
    //Extract("../tools/post.http", "../tools/rule.xml");
    //Extract("../tools/json/3.http", "../tools/json/full_oep.xml");
    Extract("../tools/gzip/2.http", "../tools/gzip/full_oep.xml");
  }

  return 0;
}


