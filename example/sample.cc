#include <string>

#include "cpp2sky/propagation.h"
#include "cpp2sky/segment_context.h"
#include "cpp2sky/tracer.h"

using namespace cpp2sky;

int main(void) {
  Config config("dummy", "instance_3", "dummy_token");
  std::string sw8 =
      "1-MQ==-NQ==-3-bWVzaA==-aW5zdGFuY2U=-L2FwaS92MS9oZWFsdGg=-"
      "ZXhhbXBsZS5jb206ODA4MA==";
  auto span_ctx = createSpanContext(sw8);
  auto current_segment = createSegmentContext(config, span_ctx);

  auto tracer = createInsecureGrpcTracer("localhost:11800");
  auto m = current_segment->createSegmentObject();

  while (true) {
    std::string s;
    std::cin >> s;
    if (s == "q") {
      break;
    } else {
      tracer->sendSegment(m);
    }
  }
}
