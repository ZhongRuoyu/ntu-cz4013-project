#ifndef DFIS_MESSAGES_INVOCATION_SEMANTIC_H_
#define DFIS_MESSAGES_INVOCATION_SEMANTIC_H_

#include <srpc/types/integers.h>

namespace dfis {

enum class InvocationSemantic : srpc::i8 {
  kAtLeastOnce = 1,
  kAtMostOnce = 2,
};

}  // namespace dfis

#endif  // DFIS_MESSAGES_INVOCATION_SEMANTIC_H_
