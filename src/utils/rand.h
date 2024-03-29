#ifndef UTILS_RAND_H_
#define UTILS_RAND_H_

#include <srpc/types/integers.h>

namespace dfis {

[[nodiscard]] srpc::u64 MakeMessageIdentifier();

}

#endif  // UTILS_RAND_H_
