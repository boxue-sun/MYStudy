#include "app_flag.h"

namespace airos {
namespace app {

DEFINE_string(
    city_string, "beij#", "city name with length is 5 and end with #");
DEFINE_string(asn_message_version, "new_4layer", "v2x message Asn.1 file version");
DEFINE_int32(xml_map_send_rate, 1000, "xml map send rate ms");
DEFINE_bool(enable_send_ssm, true, "enable send ssm");
DEFINE_bool(enable_send_map, true, "enable send map");
DEFINE_bool(enable_send_spat, true, "enable send spat");
DEFINE_bool(enable_send_rsi, true, "enable send rsi");
DEFINE_bool(enable_send_rsm, true, "enable send rsm");
DEFINE_bool(enable_print, true, "enable send rsm");


DEFINE_string(cloud_mqtt_addr, "ssl://172.16.227.84:8974", "");
DEFINE_string(cloud_mqtt_user, "v2x", "mqtt user");
DEFINE_string(cloud_mqtt_passwd, "emqx_pass", "mqtt password");
DEFINE_int32(cloud_mqtt_connect_timeout, 10, "mqtt reconnect timeout");
}  // namespace app
}  // namespace airos
