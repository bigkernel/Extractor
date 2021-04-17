// Author: yuyue/X3130 (yuyue2200@hotmail.com)

#ifndef EXTRACTOR_RULE_DEFINE_H_
#define EXTRACTOR_RULE_DEFINE_H_

#include <string>

namespace ext {
// Rule Architecture
// This file declaration the three-layer structure of OEP rule.
// ApplicationLayer: differentiate from applications
// CategoryLayer:    differentiate from rules
// RuleLayer:        a set of rule

// <RuleTree>
//   |
//   |
//   |-> [ApplicationLayer]
//   |       +-> [CategoryLayer]
//   |                +-> [RuleLayer]
//   |
//   |-> [ApplicationLayer]
//   |       +-> [CategoryLayer]
//   |                +-> [RuleLayer]
//   |
//   :

struct Protocol {
  enum Type {
    HTTP,
    TCP,
    UDP,
    UNKNOWN,
  };

  static const char* kHTTP;
  static const char* kTCP;
  static const char* kUDP;

  static Type Mapped(const std::string& s);
};

struct Codec {
  enum Type {
    GZIP,
    ZLIB,
    DEFLATE,

    URL,
    BASE64,
    UTF8,
    UNICODE,
    ESCAPE,
    QP,
    CONVERT,
    UNKNOWN,
  };

  /* Message compression method */
  static const char* kGzip;
  static const char* kZlib;
  static const char* kDeflate;

  /* Message encode method */
  static const char* kUrl;
  static const char* kBase64;
  static const char* kUtf8;
  static const char* kUnicode;
  static const char* kConvert;
  static const char* kEscape;
  static const char* kQp;

  static Type Mapped(const std::string& s);
};

struct DataSource {
  enum Type {
    URL,
    COOKIE,
    REQ_HEAD,
    REQ_CONTENT,
    RES_HEAD,
    RES_CONTENT,
    UP,
    DOWN,
    UNKNOWN,
  };

  static const char* kUri;
  static const char* kCookie;
  static const char* kReqHead;
  static const char* kReqContent;
  static const char* kResHead;
  static const char* kResContent;
  static const char* kUp;
  static const char* kDown;

  static Type Mapped(const std::string& s);
};

struct Effection {
  static const char* kActive;
  static const char* kNegative;
  static const char* kAuto;
};

struct Coordinate {
  enum Type {
    WGS84,
    GCJ02,
    BD09,
    MERCATOR,
    MAPBAR,
    SOUGOU,
    UNKNOWN,
  };

  static const char* kWgs84;
  static const char* kGcj02;
  static const char* kBd09;
  static const char* kMercator;
  static const char* kMapbar;
  static const char* kSougou;
  static const char* kUnknown;

  static Type Mapped(const std::string& s);
};

struct Skip {
  enum Type {
    ALL_DIGIT,
    ALL_LOW_LETTER,
    ALL_HIGH_LETTER,
    ALL_LETTER,
    NO_SKIP,
    UNKNOWN,
  };

  static const char* kAllDigit;
  static const char* kAllLowLetter;
  static const char* kAllHighLetter;
  static const char* kAllLetter;

  static Type Mapped(const std::string& s);
};

struct StepLayer {
  enum Type {
    // below operators
    PREFIX,
    SUFFIX,
    START_POS,
    END_POS,
    SKIP,
    RSKIP,
    LEN_LENGTH,
    SPLIT,

    // below attributes
    VALUE_ENCODE,
    ENDIAN,
    TYPE_LENGTH,
    TYPE,
    INDEX,
    UNIFY,
    FORMAT,

    KEY,
    JSON,
    JSON_HEAD,
    XML,
    XML_HEAD,
    XML_END,
    HEAD,
    TAIL,
    GROUP_SPLIT,
    WORD_SPLIT,

    UNKNOWN,
  };

  static const char* kName;

  static const char* kPrefix;
  static const char* kSuffix;
  static const char* kStartPos;
  static const char* kEndPos;
  static const char* kSkip;
  static const char* kRSkip;
  static const char* kValueEncode;
  static const char* kEndian;
  static const char* kTypeLength;
  static const char* kLengthLength;
  static const char* kIndex;
  static const char* kType;
  static const char* kSplit;
  static const char* kFormat;

  // For Json/Xml/F0/F1-rule
  static const char* kKey;

  // For Json-rule
  static const char* kJson;
  static const char* kJsonHead;

  // For XML-rule
  static const char* kXml;
  static const char* kXmlHead;
  static const char* kXmlEnd;

  // For F0/F1-rule
  static const char* kHead;       // For F0/F1-rule
  static const char* kTail;       // For F0/F1-rule
  static const char* kGroupSplit; // For F0-rule
  static const char* kWordSplit;  // For F0-rule

  static const char* kUnify;

  static Type Mapped(const std::string& s);
};

struct RuleLayer {
  enum Type {
    JSON,
    XML,
    F0,
    F1,
    UNKNOWN,
  };

  static const char* kName;
  static const char* kID;
  static const char* kKey; // {
  static const char* kJson;
  static const char* kXml;
  static const char* kF0;
  static const char* kF1;
  // }
  static const char* kDataSource;

  // The extractor will ignored to the rule when below attribute is
  // `NEGATIVE'.
  static const char* kIsEffect;

  // Layouts: YYYY-MM-DD HH:MM
  static const char* kTime;
  static const char* kPriority;
  static const char* kRuleFrom;
  static const char* kRequired;
  static const char* kDetail;

  // Extracted field character set if is not utf-8 formatted which
  // has be translated to it. this value is according to
  // iconv-style name.
  static const char* kCharacterSet;
  static const char* kConfidence;

  // Integral value to indicated that different fields of the same
  // group will only be output if they all of extracted.
  static const char* kGroup;
  static const char* kCoordinate;
  static const char* kOrigin;

  static Type Mapped(const std::string& s);
};

struct CategoryLayer {
  static const char* kName;
  static const char* kID;
};

struct ApplicationLayer {
  static const char* kName;
  static const char* kID;
  static const char* kProtocol;
  static const char* kLabel;
};

struct RuleTreeLayer {
  static const char* kName;
};

struct HttpAttributes {
  // Below application layer attributes:
  // Host            - significant

  static const char* kHost;

  // Below category layer attributes:
  // Url             - significant
  // kProtocolAction - optional
  // AppName         - optional
  // Compress        - optional
  // Encode          - optional

  static const char* kUrl;
  static const char* kProtocolAction;
  static const char* kAppName;
  static const char* kAction;

  /* Message compression position */
  static const char* kReqCntCompress;
  static const char* kResCntCompress;

  /* Message encode position */
  static const char* kResCntEncode;
  static const char* kReqCntEncode;
};

struct BinaryAttributes {
  // Below application layer attributes:
  // Host             - optional
  // IP               - significant
  // Port             - significant
  // CipherKey        - optional
  // PlaintextFeature - optional

  static const char* kHost;
  static const char* kIP;
  static const char* kPort;
  static const char* kCipherKey;
  static const char* kPlaintextFeature;

  // Below category layer attributes:
  // Url             - significant
  // kProtocolAction - optional
  // AppName         - optional
  // Action          - optional
  // Keyword         - optional
  // Compress        - optional
  // Encode          - optional

  static const char* kUrl;
  static const char* kProtocolAction;
  static const char* kAppName;
  static const char* kAction;
  static const char* kKeyword;

  /* Message compression position */
  static const char* kReqCntCompress;
  static const char* kResCntCompress;

  /* Message encode position */
  static const char* kReqCntEncode;
  static const char* kResCntEncode;
};
} // namespace ext

#endif // EXTRACTOR_RULE_DEFINE_H_
