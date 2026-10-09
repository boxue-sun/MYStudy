#include "airos_pack.h"

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

#include <jsoncpp/json/json.h>

namespace airos {
namespace pack {

AirosPack::AirosPack() {}
AirosPack::~AirosPack() {}

bool AirosPack::create(const std::string& pname, en_tpl_type type) {
  if (!check_pack_name(pname)) {
    return false;
  }

  std::string dpn(dir_create_ + pname);
  if (access(dpn.data(), F_OK) == 0) {
    std::cout << "[warn]" << dpn << " is created;delete it? please input(y/n):";
    char ch = 'y';
    if (FLAGS_ques) {
      std::cin >> ch;
    }

    if (ch == 'y') {
      std::cout << "[WARN]delete " << dpn << std::endl;
      shell_exec("rm -rf " + dpn);
    } else {
      return false;
    }
  }

  shell_exec("mkdir -p " + dpn);

  Json::Value js;
  js["name"]    = pname;
  js["version"] = "1.0.0.1";
  std::string str_type("");
  switch (type) {
    case EN_TYPE_CAMERA_PERCEPTION: {
      str_type = "camera_perception";
      break;
    }
    case EN_TYPE_LIDAR_PERCEPTION: {
      str_type = "lidar_perception";
      break;
    }
    case EN_TYPE_FUSION: {
      str_type = "fusion";
      break;
    }
    case EN_TYPE_APP: {
      str_type = "app";
      break;
    }
    case EN_TYPE_RSU: {
      str_type = "rsu";
      break;
    }
    case EN_TYPE_TRAFFIC_LIGHT: {
      str_type = "traffic_light";
      break;
    }
    case EN_TYPE_CAMERA: {
      str_type = "camera";
      break;
    }
    case EN_TYPE_LIDAR: {
      str_type = "lidar";
      break;
    }
    case EN_TYPE_RADAR: {
      str_type = "radar";
      break;
    }
    default:
      break;
  }
  js["type"] = str_type;

  std::fstream fout(
      dpn + "/version.txt", std::fstream::out | std::fstream::ate);
  if (!fout.good()) {
    std::cout << "[WARN]create version file false" << std::endl;
    return false;
  }
  fout << js.toStyledString();
  fout.close();

  CreateTemplate ct;
  ct.create(dir_create_ + pname, pname, type);

  std::cout << "[INFO]create package success: " << dir_create_ + pname
            << std::endl;
  return true;
}

bool AirosPack::make_bazel() {
  if (access("./BUILD", F_OK) != 0) {
    std::cout << "[WARN]BUILD file not exist" << std::endl;
    return false;
  }

  shell_exec("bazel build ...");

  std::string dir_out("./output");
  if (access(dir_out.data(), F_OK) == 0) {
    shell_exec("rm -rf " + dir_out);
  }

  shell_exec("mkdir -p " + dir_out);

  std::stringstream ss;
  ss << "cp ./bazel-bin/lib*.so " << dir_out << "/";
  ss << "|cp -rf ./conf " << dir_out << "/";
  ss << "|cp -rf ./version.txt " << dir_out << "/";
  ss << "|cp -rf ./lib " << dir_out << "/";
  shell_exec(ss.str());

  return true;
}

bool AirosPack::make_cmake() {
  if (access("./CMakeLists.txt", F_OK) != 0) {
    std::cout << "[WARN]CMakeLists.txt file not exist" << std::endl;
    return false;
  }
  if (access("./build", F_OK) == 0) {
    shell_exec("rm -rf ./build");
  }
  shell_exec("mkdir ./build && cd ./build && cmake .. && make");

  std::string dir_out("./output");
  if (access(dir_out.data(), F_OK) == 0) {
    shell_exec("rm -rf " + dir_out);
  }
  shell_exec("mkdir -p " + dir_out);

  std::stringstream ss;
  ss << "cp ./build/build/lib/lib*.so " << dir_out << "/";
  ss << "|cp -rf ./conf " << dir_out << "/";
  ss << "|cp -rf ./version.txt " << dir_out << "/";
  ss << "|cp -rf ./lib " << dir_out << "/";
  shell_exec(ss.str());

  return true;
}

bool AirosPack::make() {
  if (!FLAGS_name.empty()) {
    std::string dpn(dir_create_ + FLAGS_name);
    if (chdir(dpn.data()) != 0) {
      std::cout << "[WARN]change dir false: " << dpn << std::endl;
      return false;
    }
  }
  std::cout << "[INFO]make work dir: " << getcwd(nullptr, 0) << std::endl;

  std::string type = FLAGS_type;
  if (type.compare("cmake") == 0) {
    auto res = make_cmake();
    return res;
  }
  if (type.compare("bazel") == 0) {
    auto res = make_bazel();
    return res;
  }

  std::cout << "[WARN]-type is [cmake | bazel]" << std::endl;
  return false;
}

bool AirosPack::release() {
  if (!FLAGS_name.empty()) {
    std::string dpn(dir_create_ + FLAGS_name);
    if (chdir(dpn.data()) != 0) {
      std::cout << "[WARN]change dir false: " << dpn << std::endl;
      return false;
    }
  }
  std::cout << "[INFO]release work dir: " << getcwd(nullptr, 0) << std::endl;

  if (access("./output", F_OK) != 0) {
    std::cout << "[WARN]output dir not exist" << std::endl;
    return false;
  }
  if (FLAGS_out.empty()) {
    std::cout << "[WARN]release false:-out null" << std::endl;
    return false;
  }
  shell_exec("tar -zcf " + FLAGS_out + " ./output");

  std::cout << "[INFO]package is " << FLAGS_out << std::endl;

  return true;
}

bool AirosPack::install(const std::string& pfile) {
  if (access(pfile.data(), F_OK) != 0) {
    std::cout << "not exist: " << pfile << std::endl;
    return false;
  }

  std::string dir_tarx("/tmp/output");
  if (access(dir_tarx.data(), F_OK) == 0) {
    shell_exec("rm -rf " + dir_tarx);
  }
  shell_exec("tar -zxf " + pfile + " -C /tmp/");

  auto nvt = parse_verion_file("/tmp/output/version.txt");
  std::string pkg_name(nvt[0]);
  std::string pkg_ver(nvt[1]);
  std::string pkg_type(nvt[2]);

  if (pkg_name.empty()) {
    std::cout << " pkg name null";
    return false;
  }

  if (!check_pack_name(pkg_name)) {
    return false;
  }

  if (pkg_type.compare("app") == 0) {
    dir_install_ = dir_install_app_;
  }
  if (pkg_type.compare("camera_perception") == 0) {
    dir_install_ = dir_install_perception_;
  }
  if (pkg_type.compare("lidar_perception") == 0) {
    dir_install_ = dir_install_perception_;
  }
  if (pkg_type.compare("fusion") == 0) {
    dir_install_ = dir_install_fusion_;
  }
  if (pkg_type.compare("rsu") == 0) {
    dir_install_ = dir_install_rsu_;
  }
  if (pkg_type.compare("traffic_light") == 0) {
    dir_install_ = dir_install_traffic_light_;
  }
  if (pkg_type.compare("camera") == 0) {
    dir_install_ = dir_install_camera_;
  }
  if (pkg_type.compare("lidar") == 0) {
    dir_install_ = dir_install_lidar_;
  }
  if (pkg_type.compare("radar") == 0) {
    dir_install_ = dir_install_radar_;
  }
  if (!this->remove(pkg_name)) {
    // exist buf no del;
    return false;
  }

  std::stringstream ss;
  ss << "mkdir -p " << dir_install_ << pkg_name << " && echo \"\"";
  ss << "|cp -rf /tmp/output/* " << dir_install_ << pkg_name << "/";
  if (pkg_type.compare("app") == 0) {
    ss << "|cp /tmp/output/conf/app_lib_cfg.pb " << dir_install_ << pkg_name
       << "/";
  }
  if (pkg_type.compare("camera_perception") == 0 ||
      pkg_type.compare("lidar_perception") == 0 ||
      pkg_type.compare("fusion") == 0) {
    ss << "|cp /tmp/output/conf/dynamic_module.cfg " << dir_install_ << pkg_name
       << "/";
  }
  if (pkg_type.compare("rsu") == 0 || pkg_type.compare("traffic_light") == 0 ||
      pkg_type.compare("camera") == 0 || pkg_type.compare("radar") == 0 ||
      pkg_type.compare("lidar") == 0) {
    ss << "|cp /tmp/output/conf/device_lib_cfg.pb " << dir_install_ << pkg_name
       << "/";
  }

  shell_exec(ss.str());

  // del tmp
  shell_exec("rm -rf /tmp/output");
  std::cout << "[INFO]" << pkg_name << " package install in: " << dir_install_
            << std::endl;
  return true;
}

bool AirosPack::remove(const std::string& pname) {
  std::vector<std::string> vdir;
  vdir.push_back(dir_install_app_);
  vdir.push_back(dir_install_perception_);
  vdir.push_back(dir_install_fusion_);
  vdir.push_back(dir_install_rsu_);
  vdir.push_back(dir_install_traffic_light_);
  vdir.push_back(dir_install_camera_);
  vdir.push_back(dir_install_lidar_);
  vdir.push_back(dir_install_radar_);

  for (const auto& dir_install : vdir) {
    remove(dir_install, pname);
  }

  return true;
}

bool AirosPack::remove(
    const std::string& dir_install, const std::string& pname) {
  if (dir_install.empty() || pname.empty()) {
    return false;
  }

  std::string dpn(dir_install + "/" + pname);
  if (access(dpn.data(), F_OK) == 0) {
    char ch = 'y';
    if (FLAGS_ques) {
      std::cout << "[INFO]package " << dpn
                << " is installed;delete it? please input(y/n):";
      std::cin >> ch;
    }
    if (ch == 'y') {
      std::cout << "[WARN]delete have install " << dpn << std::endl;
      shell_exec("rm -rf " + dpn);

      std::string app_cfg("/home/airos/os/conf/example_app.pb");
      if (access(app_cfg.data(), F_OK) == 0) {
        shell_exec("rm -f " + app_cfg);
      }
    } else {
      return false;
    }
  }

  return true;
}

bool AirosPack::modify_run_cfg(
    const std::string& pkg_name, const std::string& pkg_type) {
  if (pkg_type.compare("app") == 0) {
    dir_install_ = dir_install_app_;
  }
  if (pkg_type.compare("camera_perception") == 0) {
    dir_install_ = dir_install_perception_;
  }
  if (pkg_type.compare("lidar_perception") == 0) {
    dir_install_ = dir_install_perception_;
  }
  if (pkg_type.compare("fusion") == 0) {
    dir_install_ = dir_install_fusion_;
  }
  if (pkg_type.compare("rsu") == 0) {
    dir_install_ = dir_install_rsu_;
  }
  if (pkg_type.compare("traffic_light") == 0) {
    dir_install_ = dir_install_traffic_light_;
  }
  if (pkg_type.compare("camera") == 0) {
    dir_install_ = dir_install_camera_;
  }
  if (pkg_type.compare("lidar") == 0) {
    dir_install_ = dir_install_lidar_;
  }
  if (pkg_type.compare("radar") == 0) {
    dir_install_ = dir_install_radar_;
  }
  std::string install_path(dir_install_ + pkg_name);
  if (access(install_path.data(), F_OK) != 0) {
    std::cout << "[WARN] package: " << pkg_name << " not installed in "
              << pkg_type << ", Please check input!" << std::endl;
    return false;
  }
  if (pkg_type.compare("app") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/app_lib_cfg.pb");
    auto m1 = cfg_read(str);
    std::map<std::string, std::string> m2;
    if (m1.count("app_name") > 0) {
      m2["app_name"] = m1["app_name"];
    }

    str                    = std::string("\"" + dir_install_ + pkg_name + "\"");
    m2["app_package_path"] = str;

    str = std::string("/home/airos/os/conf/example_app.pb");
    cfg_write(str, m2);
  }

  if (pkg_type.compare("camera_perception") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/dynamic_module.cfg");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/camera1/camera1.pt");
    auto m2 = cfg_read(run_cfg);
    if (m1.count("module_name") > 0) {
      m2["alg_name"] = m1["module_name"];
    }

    str             = std::string("\"" + dir_install_ + pkg_name + "\"");
    m2["alg_path"]  = str;
    m2["conf_path"] = str;

    cfg_write(run_cfg, m2);
  }

  if (pkg_type.compare("lidar_perception") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/dynamic_module.cfg");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/lidar1/lidar1.pt");
    auto m2 = cfg_read(run_cfg);
    if (m1.count("module_name") > 0) {
      m2["alg_name"] = m1["module_name"];
    }

    str             = std::string("\"" + dir_install_ + pkg_name + "\"");
    m2["alg_path"]  = str;
    m2["conf_path"] = str;

    cfg_write(run_cfg, m2);
  }

  if (pkg_type.compare("fusion") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/dynamic_module.cfg");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/perception_fusion_config.pt");
    std::fstream fsm(run_cfg, std::fstream::in);
    if (!fsm.good()) {
      return false;
    }
    std::stringstream ss;
    ss << fsm.rdbuf();
    fsm.close();

    fsm.open(run_cfg, std::fstream::out | std::fstream::trunc);
    if (!fsm.good()) {
      return false;
    }

    std::string str_line;
    while (getline(ss, str_line)) {
      if (str_line.find("alg_name") != str_line.npos) {
        fsm << "alg_name:" << m1["module_name"] << std::endl;
        continue;
      }
      if (str_line.find("alg_path") != str_line.npos) {
        fsm << "alg_path:" << std::string("\"" + dir_install_ + pkg_name + "\"")
            << std::endl;
        continue;
      }
      if (str_line.find("conf_path") != str_line.npos) {
        fsm << "conf_path:"
            << std::string("\"" + dir_install_ + pkg_name + "\"") << std::endl;
        continue;
      }
      fsm << str_line << std::endl;
    }
    fsm.close();
  }

  if (pkg_type.compare("rsu") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/device_lib_cfg.pb");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/rsu/config.pb.txt");
    auto m2 = cfg_read(run_cfg);
    if (m1.count("device_name") > 0) {
      m2["device"] = m1["device_name"];
    }

    cfg_write(run_cfg, m2);
  }

  if (pkg_type.compare("traffic_light") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/device_lib_cfg.pb");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/traffic_light/config.pb.txt");
    auto m2 = cfg_read(run_cfg);
    if (m1.count("device_name") > 0) {
      m2["device"] = m1["device_name"];
    }

    cfg_write(run_cfg, m2);
  }

  if (pkg_type.compare("camera") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/device_lib_cfg.pb");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/camera/camera.yaml");
    std::fstream fsm(run_cfg, std::fstream::in);
    if (!fsm.good()) {
      return false;
    }
    std::stringstream ss;
    ss << fsm.rdbuf();
    fsm.close();

    fsm.open(run_cfg, std::fstream::out | std::fstream::trunc);
    if (!fsm.good()) {
      return false;
    }

    bool b_first_find = false;
    std::string str_line;
    while (getline(ss, str_line)) {
      if (!b_first_find &&
          str_line.find("camera_manufactor") != str_line.npos) {
        b_first_find = true;
        std::string dev_name(m1["device_name"]);
        if (dev_name.front() == '\"') {
          dev_name.erase(0, 1);
        }

        if (dev_name.back() == '\"') {
          dev_name.pop_back();
        }

        fsm << "  camera_manufactor: " << dev_name << std::endl;
        continue;
      }
      fsm << str_line << std::endl;
    }
    fsm.close();
  }

  if (pkg_type.compare("lidar") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/device_lib_cfg.pb");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/lidar/lidar.yaml");
    std::fstream fsm(run_cfg, std::fstream::in);
    if (!fsm.good()) {
      return false;
    }
    std::stringstream ss;
    ss << fsm.rdbuf();
    fsm.close();

    fsm.open(run_cfg, std::fstream::out | std::fstream::trunc);
    if (!fsm.good()) {
      return false;
    }

    bool b_first_find = false;
    std::string str_line;
    while (getline(ss, str_line)) {
      if (!b_first_find && str_line.find("lidar_manufactor") != str_line.npos) {
        b_first_find = true;
        std::string dev_name(m1["device_name"]);
        if (dev_name.front() == '\"') {
          dev_name.erase(0, 1);
        }

        if (dev_name.back() == '\"') {
          dev_name.pop_back();
        }

        fsm << "  lidar_manufactor: " << dev_name << std::endl;
        continue;
      }
      fsm << str_line << std::endl;
    }
    fsm.close();
  }

  if (pkg_type.compare("radar") == 0) {
    std::string str(dir_install_ + pkg_name + "/conf/device_lib_cfg.pb");
    auto m1 = cfg_read(str);

    std::string run_cfg("/home/airos/os/conf/radar/config.pb.txt");
    auto m2 = cfg_read(run_cfg);
    if (m1.count("device_name") > 0) {
      m2["device"] = m1["device_name"];
    }

    cfg_write(run_cfg, m2);
  }

  return true;
}
bool AirosPack::run(const std::string& pname, const std::string& type) {
  if (!modify_run_cfg(pname, type)) {
    std::cout << "[WARN]modify run cfg error." << std::endl;
    return false;
  }
  std::string dpn("/home/airos/os");
  if (chdir(dpn.data()) != 0) {
    std::cout << "[WARN]change dir false:" << dpn << std::endl;
    return false;
  }
  std::cout << "[INFO]run work dir: " << getcwd(nullptr, 0) << std::endl;

  std::string cmd("");
  if (type.compare("app") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && ./bin/airos_app_framework "
        "conf/example_app.pb\"");
  }
  if (type.compare("camera_perception") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d "
        "dag/perception_camera_1.dag\"");
  }
  if (type.compare("lidar_perception") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d "
        "dag/perception_lidar_1.dag\"");
  }
  if (type.compare("fusion") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d dag/msf_component.dag\"");
  }
  if (type.compare("rsu") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d dag/rsu_service.dag\"");
  }
  if (type.compare("traffic_light") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d "
        "dag/traffic_light_service.dag\"");
  }
  if (type.compare("camera") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d "
        "dag/perception_camera_1.dag\"");
  }
  if (type.compare("lidar") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d "
        "dag/perception_lidar_1.dag\"");
  }
  if (type.compare("radar") == 0) {
    cmd = std::string(
        "bash -c \"source setup.bash && mainboard -d "
        "dag/radar_service.dag\"");
  }

  shell_exec(cmd);
  return true;
}

bool AirosPack::check_pack_name(const std::string& pname) {
  if (pname.empty() || pname.size() > 50) {
    std::cout << "[WARN]package name size [1,50] :" << pname << std::endl;
    return false;
  }

  if (!std::regex_match(pname, std::regex("^[0-9a-zA-Z_]{1,}$"))) {
    std::cout << "[WARN]package name (letters, numbers, and the underscore "
                 "character):"
              << pname << std::endl;
    return false;
  }

  return true;
}

const std::vector<std::string> AirosPack::parse_verion_file(
    const std::string& fname) {
  std::vector<std::string> vres({"", "", ""});
  if (fname.empty()) {
    return vres;
  }

  std::fstream fin(fname);
  if (!fin.good()) {
    std::cout << "version file open false: " << fname << std::endl;
    return vres;
  }

  std::stringstream ss;
  ss << fin.rdbuf();
  fin.close();
  std::string str_ver(ss.str());

  std::unique_ptr<Json::CharReader> js_reader(
      Json::CharReaderBuilder().newCharReader());
  if (!js_reader) {
    std::cout << "[WARN]js reader false" << std::endl;
    return vres;
  }

  Json::Value js;
  JSONCPP_STRING errs;
  js_reader->parse(str_ver.data(), str_ver.data() + str_ver.size(), &js, &errs);
  if (js.isNull() || !js.isMember("name") || !js["name"].isString()) {
    std::cout << "[WARN]json false" << std::endl;
    return vres;
  }

  if (js.isObject()) {
    if (js.isMember("name") && js["name"].isString()) {
      vres[0] = js["name"].asString();
    }

    if (js.isMember("version") && js["version"].isString()) {
      vres[1] = js["version"].asString();
    }

    if (js.isMember("type") && js["type"].isString()) {
      vres[2] = js["type"].asString();
    }
  }

  return vres;
}

std::map<std::string, std::string> AirosPack::cfg_read(
    const std::string& fname) {
  std::map<std::string, std::string> data;
  if (fname.empty()) {
    return data;
  }

  std::fstream fsm(fname, std::fstream::in);
  if (!fsm.good()) {
    std::cout << "[WARN]cfg read open false: " << fname << std::endl;
    return data;
  }

  std::string str;
  std::string str1;
  std::string str2;
  while (getline(fsm, str)) {
    auto pos = str.find_first_of(':');
    if (pos != str.npos) {
      str1 = stringtrim(str.substr(0, pos));
      str2 = stringtrim(str.substr(pos + 1));

      if (!str1.empty() && !str2.empty()) {
        data[str1] = str2;
      }
    }
  }
  fsm.close();

  return data;
}

void AirosPack::cfg_write(
    const std::string& fname, const std::map<std::string, std::string> data) {
  if (data.empty()) {
    return;
  }

  if (fname.empty()) {
    return;
  }

  std::fstream fsm(fname, std::fstream::out | std::fstream::trunc);
  if (!fsm.good()) {
    std::cout << "[WARN]cfg write open false: " << fname << std::endl;
    return;
  }

  for (const auto& it : data) {
    fsm << it.first << " : " << it.second << std::endl;
  }
  fsm.close();

  return;
}

std::string AirosPack::stringtrim(const std::string& str) {
  std::string::size_type start = str.find_first_not_of(" \r\n\t");
  if (start == std::string::npos) {
    return "";
  }
  std::string::size_type end = str.find_last_not_of(" \r\n\t");
  if (end == std::string::npos) {
    return "";
  }
  if (end - start + 1 < 1) {
    return "";
  }
  return str.substr(start, end - start + 1);
}

bool AirosPack::shell_exec(const std::string& cmd) {
  if (cmd.empty()) {
    return false;
  }

  if (system(cmd.data()) < 0) {
    std::cout << "[WARN]cmd false: " << cmd << std::endl;
    return false;
  }

  return true;
}

}  // namespace pack
}  // namespace airos
