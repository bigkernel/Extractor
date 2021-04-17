// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include <iostream>
#include <fstream>
#include "extractor/fhmf.h"

void ReadFile(const char* fname, std::string* out) {
  char tmp[4096];
  size_t size = 0;
  std::ifstream in(fname);
  if (in) {
    while ((size = in.readsome(tmp, 4096)) > 0)
      out->append(tmp, size);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << argv[0] << " file.fhmf" << std::endl;
    return 0;
  }

  std::string buf;
  ReadFile(argv[1], &buf);
  ext::Fhmf res;
  bool ret = ext::ParseFhmf(buf.data(), buf.size(), &res);
  if (!ret) {
    std::cout << "Invalid FHMF file" << std::endl;
    return -1;
  }

  std::cout << "Version " << res.version << std::endl << std::endl;
  for (size_t i = 0; i < res.fields.size(); ++i) {
    auto& field = res.fields[i];
    for (auto iter = field.options.begin(); iter != field.options.end(); ++iter) {
      std::cout << iter->first << " - "
                << iter->second << std::endl;
    }
    std::cout.write(field.payload_ptr, field.payload_len) << std::endl;
  }

  return 0;
}

