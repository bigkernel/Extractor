
TARGET=rule_test extractor_test filter_test codec_test fhmf_test message_test

all: $(TARGET);

rule_test: rule_test.cc rule.cc rule_define.cc rule_ops.cc trivial.cc filter.cc third_party/string_view.cc
	g++ -std=c++0x -g -Wall -Wextra -Werror -rdynamic -I.. -o $@ $^ -lexpat -lz
	
codec_test: codec_test.cc codec.cc trivial.cc rule_define.cc
	g++ -std=c++0x -g -Wall -Wextra -Werror -rdynamic -I.. -o $@ $^ -lexpat -lz

extractor_test: extractor_test.cc \
		extractor.cc \
		rule.cc \
		rule_define.cc \
		rule_ops.cc \
		parser.cc \
		codec.cc \
		fhmf.cc \
		filter.cc \
		message.cc \
		http_parser1.cc \
		binary_parser.cc \
		third_party/http_parser.c \
		third_party/string_view.cc \
		third_party/jsoncpp.cc \
		trivial.cc
	g++ -std=c++0x -g -O2 -Wall -Wextra -Werror -rdynamic -I.. -o $@ $^ -lexpat -lz -lpthread

filter_test: filter_test.cc filter.cc
	g++ -std=c++0x -g -Wall -Wextra -Werror -rdynamic -I.. -o $@ $^ -lexpat -lz
	
fhmf_test: fhmf_test.cc fhmf.cc
	g++ -std=c++0x -g -Wall -Wextra -Werror -rdynamic -I.. -o $@ $^
	
message_test: message_test.cc message.cc fhmf.cc rule_define.cc trivial.cc third_party/http_parser.c
	g++ -std=c++0x -g -Wall -Wextra -Werror -rdynamic -I.. -o $@ $^

clean:
	rm -rf *.o $(TARGET)
