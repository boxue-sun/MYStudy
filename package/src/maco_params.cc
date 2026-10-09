#include "maco_params.h"

namespace airos {
namespace pack {

DEFINE_string(name, "", "package name");
DEFINE_string(type, "", "perception,app,device");
DEFINE_string(file, "", "package for install");
DEFINE_bool(ques, false, "del question");
DEFINE_string(out, "output.tar.gz", "release out name");
DEFINE_bool(h, false, "");

}  // namespace pack
}  // namespace airos
