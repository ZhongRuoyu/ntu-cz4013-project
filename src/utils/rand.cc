#include "utils/rand.h"

#include <limits>
#include <random>

#include <srpc/types/integers.h>

namespace dfis {

srpc::u64 MakeMessageIdentifier() {
  static std::random_device rand;
  return std::uniform_int_distribution<srpc::u64>{
      std::numeric_limits<srpc::u64>::min(),
      std::numeric_limits<srpc::u64>::max()}(rand);
}

}  // namespace dfis
