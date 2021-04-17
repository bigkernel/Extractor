// Author: yuyue/X3130 (X3130@njsecnet.com)

#include <iostream>
#include <fstream>
#include "extractor/message.h"

using namespace ext;

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

  Message msg = Probe(buf.data(), buf.size());
  if (msg.type == Protocol::Type::UNKNOWN) {
    std::cout << "Unknown message" << std::endl;
    return -1;
  }

  std::cout << "Message type: " << msg.type << std::endl;
  switch (msg.type) {
  case Protocol::Type::HTTP: {
    for (int i = Message::Slice::HTTP_HOST;
        i < Message::Slice::HTTP_RES + 1; ++i) {
      std::cout << Slice(&msg, Message::Slice::Type(i)).str
          << std::endl;
    }
    break;
  }
  case Protocol::Type::TCP:
  case Protocol::Type::UDP:
    for (int i = Message::Slice::BIN_DOMAIN;
        i < Message::Slice::BIN_RES + 1; ++i) {
      std::cout << Slice(&msg, Message::Slice::Type(i)).str
          << std::endl;
    }
    break;
  default: break;
  }
  return 0;
}

