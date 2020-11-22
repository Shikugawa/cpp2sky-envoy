// Copyright 2020 SkyAPM

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/tracer_impl.h"

#include "source/utils/exception.h"

namespace cpp2sky {

TracerImpl::TracerImpl(std::string address,
                       std::shared_ptr<grpc::ChannelCredentials> cred,
                       GrpcAsyncSegmentReporterStreamFactory& factory)
    : th_([this] { this->run(); }) {
  client_ = new GrpcAsyncSegmentReporterClient(&cq_, factory, cred, address);
}

TracerImpl::~TracerImpl() {
  delete client_;
  th_.join();
  cq_.Shutdown();
}

void TracerImpl::run() {
  void* got_tag;
  bool ok = false;
  while (true) {
    gpr_timespec deadline = gpr_time_add(
        gpr_now(GPR_CLOCK_REALTIME), gpr_time_from_seconds(3, GPR_TIMESPAN));
    grpc::CompletionQueue::NextStatus status =
        cq_.AsyncNext(&got_tag, &ok, deadline);
    if (status == grpc::CompletionQueue::SHUTDOWN) {
      return;
    }
    if (status == grpc::CompletionQueue::NextStatus::TIMEOUT) {
      continue;
    }
    TaggedStream* t_stream = deTag(got_tag);
    if (!ok || !t_stream->stream->handleOperation(t_stream->operation)) {
      return;
    }
  }
}

TracerPtr createInsecureGrpcTracer(std::string address) {
  return std::make_unique<TracerImpl>(
      address, grpc::InsecureChannelCredentials(), stream_factory);
}

}  // namespace cpp2sky
