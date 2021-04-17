// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#include "extractor/rule_define.h"
#include <unordered_map>

namespace ext {
#define MAKE_MAPPED_OPS(map, key)   \
  auto iter = map.find(key);        \
  if (iter != map.cend())           \
    return iter->second;            \
  return Type::UNKNOWN;

// Protocol {
const char* Protocol::kHTTP = "HTTP";
const char* Protocol::kTCP = "TCP";
const char* Protocol::kUDP = "UDP";

Protocol::Type Protocol::Mapped(const std::string& s) {
  static const std::unordered_map<std::string, Protocol::Type> map{
      { Protocol::kHTTP, Protocol::Type::HTTP  },
      { Protocol::kTCP,  Protocol::Type::TCP   },
      { Protocol::kUDP,  Protocol::Type::UDP   },
  };
  MAKE_MAPPED_OPS(map, s)
}
// }

// CodecMethod {
const char* Codec::kGzip      = "GZIP";
const char* Codec::kZlib      = "ZLIB";
const char* Codec::kDeflate   = "DEFLATE";

const char* Codec::kUrl       = "URL";
const char* Codec::kBase64    = "BASE64";
const char* Codec::kUtf8      = "UTF-8";
const char* Codec::kUnicode   = "UNICODE";
const char* Codec::kConvert   = "CONVERT";
const char* Codec::kEscape    = "ESCAPE";
const char* Codec::kQp        = "QP";

Codec::Type Codec::Mapped(const std::string& s) {
  static const std::unordered_map<std::string, Codec::Type> map{
      { Codec::kGzip,     Codec::Type::GZIP     },
      { Codec::kZlib,     Codec::Type::ZLIB     },
      { Codec::kDeflate,  Codec::Type::DEFLATE  },

      { Codec::kUrl,      Codec::Type::URL      },
      { Codec::kBase64,   Codec::Type::BASE64   },
      { Codec::kUtf8,     Codec::Type::UTF8     },
      { Codec::kUnicode,  Codec::Type::UNICODE  },
      { Codec::kConvert,  Codec::Type::CONVERT  },
      { Codec::kEscape,   Codec::Type::ESCAPE   },
      { Codec::kQp,       Codec::Type::QP       },
  };
  MAKE_MAPPED_OPS(map, s)
}
// }

// DataSource {
const char* DataSource::kUri          = "URI";
const char* DataSource::kCookie       = "COOKIE";
const char* DataSource::kReqHead      = "REQUESTHEAD";
const char* DataSource::kReqContent   = "REQUESTCONTENT";
const char* DataSource::kResHead      = "RESPONSEHEAD";
const char* DataSource::kResContent   = "RESPONSECONTENT";
const char* DataSource::kUp           = "UP";
const char* DataSource::kDown         = "DOWN";

DataSource::Type DataSource::Mapped(const std::string& s) {
  static const std::unordered_map<std::string, DataSource::Type> map{
      { DataSource::kUri,         DataSource::Type::URL         },
      { DataSource::kCookie,      DataSource::Type::COOKIE      },
      { DataSource::kReqHead,     DataSource::Type::REQ_HEAD    },
      { DataSource::kReqContent,  DataSource::Type::REQ_CONTENT },
      { DataSource::kResHead,     DataSource::Type::RES_HEAD    },
      { DataSource::kResContent,  DataSource::Type::RES_CONTENT },
      { DataSource::kUp,          DataSource::Type::REQ_CONTENT },
      { DataSource::kDown,        DataSource::Type::RES_CONTENT },
  };
  MAKE_MAPPED_OPS(map, s)
}
// }

// Effection {
const char* Effection::kActive    = "ACTIVE";
const char* Effection::kNegative  = "NEGATIVE";
const char* Effection::kAuto      = "AUTO";
// }

// Coordinate {
const char* Coordinate::kWgs84    = "WGS84_COORDINATE";
const char* Coordinate::kGcj02    = "GCJ02_COORDINATE";
const char* Coordinate::kBd09     = "BD09_COORDINATE";
const char* Coordinate::kMercator = "MERCATOR_COORDINATE";
const char* Coordinate::kMapbar   = "MAPBAR_COORDINATE";
const char* Coordinate::kSougou   = "SOUGOU_COORDINATE";
const char* Coordinate::kUnknown  = "UNK_COORDINATE";

Coordinate::Type Coordinate::Mapped(const std::string& s) {
  static const std::unordered_map<std::string, Coordinate::Type> map{
      { Coordinate::kWgs84,   Coordinate::Type::WGS84   },
      { Coordinate::kGcj02,   Coordinate::Type::GCJ02   },
      { Coordinate::kBd09,    Coordinate::Type::BD09    },
      { Coordinate::kMercator,Coordinate::Type::MERCATOR},
      { Coordinate::kMapbar,  Coordinate::Type::MAPBAR  },
      { Coordinate::kSougou,  Coordinate::Type::SOUGOU  },
      { Coordinate::kUnknown, Coordinate::Type::UNKNOWN },
  };
  MAKE_MAPPED_OPS(map, s)
}
// }

// Skip {
const char* Skip::kAllDigit       = "all-Digit";
const char* Skip::kAllLowLetter   = "all-LowLetter";
const char* Skip::kAllHighLetter  = "all-HighLetter";
const char* Skip::kAllLetter      = "all-Letter";

Skip::Type Skip::Mapped(const std::string& s) {
  static const std::unordered_map<std::string, Skip::Type> map{
      { Skip::kAllDigit,      Skip::Type::ALL_DIGIT       },
      { Skip::kAllLowLetter,  Skip::Type::ALL_LOW_LETTER  },
      { Skip::kAllHighLetter, Skip::Type::ALL_HIGH_LETTER },
      { Skip::kAllLetter,     Skip::Type::ALL_LETTER      },
  };
  MAKE_MAPPED_OPS(map, s)
}
// }

// StepLayer {
const char* StepLayer::kName          = "STEP";
const char* StepLayer::kPrefix        = "Prefix";
const char* StepLayer::kSuffix        = "Suffix";
const char* StepLayer::kStartPos      = "StartPos";
const char* StepLayer::kEndPos        = "EndPos";
const char* StepLayer::kSkip          = "Skip";
const char* StepLayer::kRSkip         = "RSkip";
const char* StepLayer::kValueEncode   = "ValueEncode";
const char* StepLayer::kEndian        = "Endian";
const char* StepLayer::kTypeLength    = "TLen";
const char* StepLayer::kLengthLength  = "LLen";
const char* StepLayer::kIndex         = "Index";
const char* StepLayer::kType          = "Type";
const char* StepLayer::kSplit         = "Split";
const char* StepLayer::kFormat        = "Format";
const char* StepLayer::kKey           = "Key";
const char* StepLayer::kJson          = "Json";
const char* StepLayer::kJsonHead      = "JsonHead";
const char* StepLayer::kXml           = "Xml";
const char* StepLayer::kXmlHead       = "XmlHead";
const char* StepLayer::kXmlEnd        = "XmlEnd";
const char* StepLayer::kHead          = "Head";
const char* StepLayer::kTail          = "Tail";
const char* StepLayer::kGroupSplit    = "GroupSplit";
const char* StepLayer::kWordSplit     = "WordSplit";
const char* StepLayer::kUnify         = "Unify";

StepLayer::Type StepLayer::Mapped(const std::string& s) {
  static const std::unordered_map<std::string, StepLayer::Type> map{
      { StepLayer::kPrefix,       StepLayer::Type::PREFIX       },
      { StepLayer::kSuffix,       StepLayer::Type::SUFFIX       },
      { StepLayer::kStartPos,     StepLayer::Type::START_POS    },
      { StepLayer::kEndPos,       StepLayer::Type::END_POS      },
      { StepLayer::kSkip,         StepLayer::Type::SKIP         },
      { StepLayer::kRSkip,        StepLayer::Type::RSKIP        },
      { StepLayer::kValueEncode,  StepLayer::Type::VALUE_ENCODE },
      { StepLayer::kEndian,       StepLayer::Type::ENDIAN       },
      { StepLayer::kTypeLength,   StepLayer::Type::TYPE_LENGTH  },
      { StepLayer::kLengthLength, StepLayer::Type::LEN_LENGTH   },
      { StepLayer::kIndex,        StepLayer::Type::INDEX        },
      { StepLayer::kType,         StepLayer::Type::TYPE         },
      { StepLayer::kSplit,        StepLayer::Type::SPLIT        },
      { StepLayer::kFormat,       StepLayer::Type::FORMAT       },
      { StepLayer::kUnify,        StepLayer::Type::UNIFY        },
      { StepLayer::kKey,          StepLayer::Type::KEY          },
      { StepLayer::kJson,         StepLayer::Type::JSON         },
      { StepLayer::kJsonHead,     StepLayer::Type::JSON_HEAD    },
      { StepLayer::kXml,          StepLayer::Type::XML          },
      { StepLayer::kXmlHead,      StepLayer::Type::XML_HEAD     },
      { StepLayer::kXmlEnd,       StepLayer::Type::XML_END      },
      { StepLayer::kHead,         StepLayer::Type::HEAD         },
      { StepLayer::kTail,         StepLayer::Type::TAIL         },
      { StepLayer::kGroupSplit,   StepLayer::Type::GROUP_SPLIT  },
      { StepLayer::kWordSplit,    StepLayer::Type::WORD_SPLIT   },
  };
  MAKE_MAPPED_OPS(map, s)
}
// }

// RuleLayer {
const char* RuleLayer::kName          = "RULE";
const char* RuleLayer::kID            = "RuleId";

const char* RuleLayer::kKey           = "Key"; // {
const char* RuleLayer::kJson          = "JSON";
const char* RuleLayer::kXml           = "XML";
const char* RuleLayer::kF0            = "F0";
const char* RuleLayer::kF1            = "F1";
// }
const char* RuleLayer::kDataSource    = "DataSource";
const char* RuleLayer::kIsEffect      = "IsEffect";
const char* RuleLayer::kTime          = "Time";
const char* RuleLayer::kPriority      = "Priority";
const char* RuleLayer::kRuleFrom      = "RuleFrom";
const char* RuleLayer::kRequired      = "Required";
const char* RuleLayer::kDetail        = "Detail";
const char* RuleLayer::kCharacterSet  = "CharacterSet";
const char* RuleLayer::kConfidence    = "Confidence";
const char* RuleLayer::kGroup         = "Group";
const char* RuleLayer::kCoordinate    = "Coordinate";
const char* RuleLayer::kOrigin        = "Origin";

RuleLayer::Type RuleLayer::Mapped(const std::string& s) {
  static const std::unordered_map<std::string, RuleLayer::Type> map{
      { RuleLayer::kJson,  RuleLayer::Type::JSON },
      { RuleLayer::kXml,   RuleLayer::Type::XML  },
      { RuleLayer::kF0,    RuleLayer::Type::F0   },
      { RuleLayer::kF1,    RuleLayer::Type::F1   },
  };
  MAKE_MAPPED_OPS(map, s)
}
// }

// CategoryLayer {
const char* CategoryLayer::kName      = "URL";
const char* CategoryLayer::kID        = "UrlId";
// }

// ApplicationLayer {
const char* ApplicationLayer::kName   = "HOST";
const char* ApplicationLayer::kID     = "HostId";
const char* ApplicationLayer::kProtocol = "Protocol";
const char* ApplicationLayer::kLabel  = "EvilLabel";
// }

// RuleTreeLayer {
const char* RuleTreeLayer::kName      = "pIE_RULES";
// }

// HttpAttributes
const char* HttpAttributes::kHost           = "Host";
const char* HttpAttributes::kUrl            = "Url";
const char* HttpAttributes::kProtocolAction = "ProtocolAction";
const char* HttpAttributes::kAppName        = "AppName";
const char* HttpAttributes::kAction         = "Action";
// Message compression position
const char* HttpAttributes::kReqCntCompress = "ReqCntCompress";
const char* HttpAttributes::kResCntCompress = "ResCntCompress";
// Message encode position
const char* HttpAttributes::kResCntEncode   = "ResCntEncode";
const char* HttpAttributes::kReqCntEncode   = "ReqCntEncode";

// BinaryAttributes
const char* BinaryAttributes::kHost           = "Host";
const char* BinaryAttributes::kIP             = "Ip";
const char* BinaryAttributes::kPort           = "Port";
const char* BinaryAttributes::kCipherKey      = "CipherKey";
const char* BinaryAttributes::kPlaintextFeature = "PlaintextFeature";
const char* BinaryAttributes::kUrl            = "Url";
const char* BinaryAttributes::kProtocolAction = "ProtocolAction";
const char* BinaryAttributes::kAppName        = "AppName";
const char* BinaryAttributes::kAction         = "Action";
const char* BinaryAttributes::kKeyword        = "Keyword";
// Message compression position
const char* BinaryAttributes::kReqCntCompress = "ReqCntCompress";
const char* BinaryAttributes::kResCntCompress = "ResCntCompress";
// Message encode position
const char* BinaryAttributes::kResCntEncode   = "ResCntEncode";
const char* BinaryAttributes::kReqCntEncode   = "ReqCntEncode";

} // namespace ext
