#include "utils/rand.h"

#include <limits>
#include <random>

#include <srpc/types/integers.h>

namespace dfis {

srpc::i64 MakeMessageIdentifier() {
  static std::random_device rand;
  return std::uniform_int_distribution<srpc::i64>{
      std::numeric_limits<srpc::i64>::min(),
      std::numeric_limits<srpc::i64>::max()}(rand);
}

}  // namespace dfis
