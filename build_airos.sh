#! /usr/bin/env bash
set -e

TOP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)" # 得到当前脚本执行的目录

ARCH="$(uname -m)"
SUPPORTED_ARCHS=" x86_64 aarch64 "

SUPPORTED_DEVICE_SERVICE=(traffic_light rsu camera lidar mec ipcamera )
#############################################################
protocol_version="v3.3"
RELEASE_ROOT="/home/airos"
package_version="v2.3.1"

# 设置项目的名称
package_project="shunyi-auth"
#根据test_switch取值进行判断，test_switch=0 ，取值为release;test_switch=1 ，取值为 debug
package_name=""
#1 打包 0不打包
tar_package_switch="1"
#拷贝，1拷贝， 0 不拷贝
scp_remote="0"
#1 测试 0版本
test_switch="0"

host_ip="172.20.65.176"
user_name="root"
user_passwd="cictci@2024"
scp_dist_dir="/home/airos"
#############################################################
OUT_ROOT="${RELEASE_ROOT}/os"
OUT_3RD="${OUT_ROOT}/3rd"
OUT_DAG="${OUT_ROOT}/dag"
OUT_CONF="${OUT_ROOT}/conf"
OUT_LIB="${OUT_ROOT}/lib"
OUT_BIN="${OUT_ROOT}/bin"

device_service_path="middleware/device_service/framework"
device_modules_path="middleware/device_service/modules"


OUT_OS="${RELEASE_ROOT}/os"
OUT_PARAM="${RELEASE_ROOT}/param"
OUT_PROTOCOL="${RELEASE_ROOT}/protocol"
# 检查 libgeographic-dev 是否安装
# if ! dpkg-query -W -f='${Status}' libgeographic-dev 2>/dev/null | grep -q "^install"; then
#     echo "libgeographic-dev is not installed. Installing it..."
#     sudo apt-get install -y --no-install-recommends libgeographic-dev
# fi
function del_out_dir()
{
  rm -rf ${OUT_OS}
  rm -rf ${OUT_PARAM}
  rm -rf ${OUT_PROTOCOL}
}
function generate_version_file()
{
    version_path="/home/airos/os/.airos_version"
    if [ ! -e "${so_path}" ];then
      current_date=$(date "+%Y%m%d%H%M")
      airos_version="airos-${protocol_version}-${package_version}-${package_project}-${package_name}-${current_date}"

      output_file="${airos_version}.tar.gz"
      # 将版本号, 写入配置文件中
      echo "${airos_version}" > "${version_path}"
    fi
}
function tar_package()
{
  if [ ${tar_package_switch} = "1" ]; then
       echo "Start pack ... "
       # 设置要打包的目录
       if [ ${test_switch} = "1" ]; then
          echo "this pack --> debug"
          package_name="debug"
          dirs_os="os"
          dirs_parm="param"
          dirs_protocol="protocol"
          dirs_common_config="common_config/v2x_rsi_config.pb common_config/airos_cron_conf common_config/airos_log_conf common_config/rsu_map.xml common_config/xer_map.xml common_config/PTPmonitor_conf"
          dirs_script="script"

#          python_pack="need_package"
       else
          echo "this pack --> release"
          package_name="release"
          dirs_os="os"
          dirs_parm="param"
          dirs_protocol="protocol"
          dirs_common_config="common_config/v2x_rsi_config.pb common_config/airos_cron_conf common_config/airos_log_conf common_config/rsu_map.xml common_config/xer_map.xml common_config/PTPmonitor_conf"
          dirs_script="script"
#          python_pack="need_package"
        fi
          # 设置输出文件名
#          current_date=$(date "+%Y%m%d%H%M")
#          airos_version="airos-${package_version}-${package_name}-${current_date}"
#          output_file="${airos_version}.tar.gz"
#          output_path="/home/airos/${output_file}"
#          version_path="/home/airos/os/.airos_version"
#          # 将版本号, 写入配置文件中
#          echo "${airos_version}" > "${version_path}"
          generate_version_file
          remote_host="${user_name}@${host_ip}:${scp_dist_dir}"
          # 进入临时目录并打包
          cd "/home/airos/"
          # 检查目录下是否有.tar.gz文件
          if [ "$(ls -1 *.tar.gz 2>/dev/null | wc -l)" -gt 0 ]; then
              # 有.tar.gz文件,删除它们
              echo "Deleting .tar.gz files in /home/airos/"
              rm ./*.tar.gz
          else
              echo "No .tar.gz files found in /home/airos/"
          fi
          tar zcvf ${output_file} ${dirs_os} ${dirs_parm} ${dirs_protocol} ${dirs_common_config} ${dirs_script} ${python_pack}

          if [ ${scp_remote} = "1" ]; then
                echo "sshpass start!"
                sshpass -p ${user_passwd} scp -P 2222 "${output_path}" "${remote_host}"
                echo "[scp]sucess!: ${output_path}"
          else
                # mv "${output_path}" "/airos"
                echo "[mv]mv tar sucess!: ${output_path}"
          fi
  fi
}


function check_architecture_support() {
    if [[ "${SUPPORTED_ARCHS}" != *" ${ARCH} "* ]]; then
        echo "Unsupported CPU arch: ${ARCH}. Currently, AIROS only" \
            "supports running on the following CPU archs:"
        echo "${TAB}${SUPPORTED_ARCHS}"
        exit 1
    fi
}

function check_platform_support() {
    local platform="$(uname -s)"
    if [[ "${platform}" != "Linux" ]]; then
        echo "Unsupported platform: ${platform}."
        echo "${TAB}AIROS is expected to run on Linux systems (E.g., Debian/Ubuntu)."
        exit 1
    fi
}

function check_minimal_memory_requirement() {
    local minimal_mem_gb="2.0"
    local actual_mem_gb="$(free -m | awk '/Mem:/ {printf("%0.2f", $2 / 1024.0)}')"
    if (($(echo "$actual_mem_gb < $minimal_mem_gb" | bc -l))); then
        echo "System memory [${actual_mem_gb}G] is lower than the minimum required" \
            "[${minimal_mem_gb}G]. AIROS build could fail."
    fi
}

function env_setup() {
    check_architecture_support
    check_platform_support
    check_minimal_memory_requirement
}

function run_base_device_connect_ut() {
    local support_device=(rsu traffic_light ins radar lidar mec ipcamera)
    for device in ${support_device[@]}
    do
        pushd ./bazel-bin/${device_modules_path}/${device} > /dev/null
            ./${device}_ut
        popd > /dev/null
    done
    echo "base device connect ut done!"
}

function release_middleware_device_service() {
    for device in ${SUPPORTED_DEVICE_SERVICE[@]}
    do
        local so_path="${TOP_DIR}/bazel-bin/${device_service_path}/${device}/lib${device}_component.so"
        if [ -e "${so_path}" ];then
            cp -f ${so_path} ${OUT_LIB}
        fi
        local dag_path="${TOP_DIR}/${device_service_path}/${device}/dag"
        if [ -d "${dag_path}" ];then
            cp -rf ${dag_path}/* ${OUT_DAG}
        fi

        # framework conf
        device_framework_cfg_path=${OUT_CONF}/${device}
        [ -d "${device_framework_cfg_path}" ] || mkdir -p ${device_framework_cfg_path}
        if [ "${device}" = "camera" ] || [ "${device}" = "lidar" ]; then
          if [ ! -e "${OUT_CONF}/${device}/${device}.yaml" ]; then
            ln -s ${RELEASE_ROOT}/param/device/${device}/${device}.yaml ${OUT_CONF}/${device}/${device}.yaml
          fi   
        else
          local conf_path="${TOP_DIR}/${device_service_path}/${device}/conf/config.pb.txt"
          if [ -e "${conf_path}" ];then
              cp -rf ${conf_path} ${device_framework_cfg_path}
          fi
        fi

    done
}

function release_middleware_protocol(){
    local codec_path="middleware/protocol/v2x_codec"
    local so_path="${TOP_DIR}/bazel-bin/${codec_path}/libv2x_codec.so"
    if [ -e "${so_path}" ];then
        cp -f ${so_path} ${OUT_LIB}
    fi

    local conf_path="${TOP_DIR}/${codec_path}/conf/v2x_codec.flag"
    cp -f ${conf_path} ${OUT_CONF}

    local dag_path="${TOP_DIR}/${codec_path}/dag"
    if [ -d "${dag_path}" ];then
        cp -rf ${dag_path}/* ${OUT_DAG}
    fi

#    local support_protocol=(om ccindex radar_point_cloud  om_common om_device_status om_mec om_radar om_camera om_cloud rsap bs_angle_offset \
#                         bs_spat_src_data bs_v2x_data  bs_v2x_bsm_data camera_event om_monitor radar_static radar_traffic_metrics radar_tc radar_cloud)

    local support_protocol=(om ccindex rsap radar_point_cloud  om_common om_device_status om_monitor spill_reporter sound_player)
    local support_protocol_radar_point_cloud=(monitor_mec)
    local release_protocol_path="${RELEASE_ROOT}/protocol"
    for protocol in ${support_protocol[@]}
    do
          local codec_path="middleware/protocol/${protocol}"
          if [ "${protocol}" = "om" ];then
              [ -d "${release_protocol_path}/${protocol}" ] || mkdir -p ${release_protocol_path}/${protocol}
              local release_protocol_module_path="${release_protocol_path}/${protocol}"
              local conf_path="${TOP_DIR}/${codec_path}"
              cp -rf ${conf_path}/*.py "${release_protocol_module_path}"
          fi

          if [ "${protocol}" = "radar_point_cloud" ];then
              [ -d "${release_protocol_path}/${protocol}" ] || mkdir -p ${release_protocol_path}/${protocol}
              local release_protocol_module_path="${release_protocol_path}/${protocol}"
              local conf_path="${TOP_DIR}/${codec_path}"
              cp -rf ${conf_path}/* "${release_protocol_module_path}"
              for depend_proto_py_file_name in ${support_protocol_radar_point_cloud[@]}
              do
                local depend_proto_py_file_path="${TOP_DIR}/bazel-bin/middleware/protocol/proto/${depend_proto_py_file_name}*.py"
                cp ${depend_proto_py_file_path} "${release_protocol_module_path}"
              done
          else
             local so_path="${TOP_DIR}/bazel-bin/${codec_path}/lib${protocol}.so"
              if [ -e "${so_path}" ];then
                  cp -f ${so_path} ${OUT_LIB}
              fi
              if [ "${protocol}" = "rsap" ];then
                  [ -d "${release_protocol_path}/${protocol}" ] || mkdir -p ${release_protocol_path}/${protocol}
                  local release_protocol_module_path="${release_protocol_path}/${protocol}"
                  local conf_path="${TOP_DIR}/${codec_path}/conf"
                  if [ -d "${dag_path}" ];then
                      [ -d "${release_protocol_module_path}/conf" ] || mkdir -p "${release_protocol_module_path}/conf"
                      cp -rf ${conf_path}/* "${release_protocol_module_path}/conf"
                  fi
              fi


              local dag_path="${TOP_DIR}/${codec_path}/dag"
              if [ -d "${dag_path}" ];then
                  cp -rf ${dag_path}/* ${OUT_DAG}
              fi
          fi
    done

}

function copy_lib_from_dir() {
   for file in ` ls $1`
   do
       if [ -d $1"/"$file ]
       then
            if [ "${file##*.}"x != "runfiles"x ]
            then
                copy_lib_from_dir $1"/"$file $2
            fi
       else
            if [ "${file##*.}"x = "so"x ]
            then
            local path="$1/$file"
            local name=$file
            if [ ! -f $2"/"$name ]
            then
                cp -f ${path} "$2/${name}"
            fi
           fi
       fi
   done
}

function release_framework() {
    copy_framework() {
        local framework_path="$1"
        local out_lib="$2"
        local out_dag="$3"
        local out_conf="$4"

        copy_lib_from_dir "${TOP_DIR}/bazel-bin/${framework_path}" "${out_lib}"
        cp -rf ${TOP_DIR}/${framework_path}/dag/* "${out_dag}"
        cp -rf ${TOP_DIR}/${framework_path}/conf/* "${out_conf}"
    }
    local perception_camera_framework_path="air_service/framework/perception-camera"
    copy_framework "${perception_camera_framework_path}" "${OUT_LIB}" "${OUT_DAG}" "${OUT_CONF}"

    local perception_fusion_framework_path="air_service/framework/perception-fusion"
    copy_framework "${perception_fusion_framework_path}" "${OUT_LIB}" "${OUT_DAG}" "${OUT_CONF}"

    local perception_lidar_framework_path="air_service/framework/perception-lidar"
    copy_framework "${perception_lidar_framework_path}" "${OUT_LIB}" "${OUT_DAG}" "${OUT_CONF}"
}

function release_algo_modules() {
    local OUT_MODULES_LIB="${OUT_ROOT}/modules/lib"
    local OUT_MODULES_CONF="${OUT_ROOT}/modules/conf"
    copy_module() {
        local module_path="$1"
        local module_name="$2"
        local out_path="${OUT_MODULES_LIB}/airos_${module_name}"

        [ -d "${out_path}" ] || mkdir -p "${out_path}"
        copy_lib_from_dir "${TOP_DIR}/bazel-bin/${module_path}" "${out_path}"

        cp -rf "${TOP_DIR}/${module_path}/conf" "${out_path}"
        mv "${out_path}/conf/dynamic_module.cfg" "${out_path}"
    }
    copy_module "air_service/modules/perception-camera" "perception_camera"
    copy_module "air_service/modules/perception-fusion" "perception_fusion"
    copy_module "air_service/modules/perception-lidar" "perception_lidar"

    # release model
    local perception_camera_out="${OUT_MODULES_LIB}/airos_perception_camera"
    local perception_camera_path="air_service/modules/perception-camera"
    [ -d "${perception_camera_out}/data" ] || mkdir -p "${perception_camera_out}/data"
    cp -rf "${TOP_DIR}/${perception_camera_path}/algorithm/detector/air_detector/data" "${perception_camera_out}/data/detector/"
    cp -rf "${TOP_DIR}/${perception_camera_path}/algorithm/tracker/air_tracker/data" "${perception_camera_out}/data/tracker/"

    # viz conf
    local viz_conf=${OUT_CONF}/viz
    [ -d "${viz_conf}" ] || mkdir -p "${viz_conf}"
    cp air_service/modules/perception-visualization/config/*.{ini,pt} ${viz_conf}
}

function release_common_param() {
  for dir in ${SUPPORTED_DEVICE_SERVICE[@]}
  do
    [ -d "${RELEASE_ROOT}/param/device/${dir}" ] || mkdir -p ${RELEASE_ROOT}/param/device/${dir}
  done

  for device in ${SUPPORTED_DEVICE_SERVICE[@]}
  do
    cp -rf ${TOP_DIR}/${device_service_path}/${device}/conf/* ${RELEASE_ROOT}/param/device/${device}
  done
}

function release_services() {
    release_framework
    release_algo_modules
    
    # release launch
    # cp -rf ${TOP_DIR}/air_service/launch ${OUT_ROOT}
}

function release_middleware() {
    release_middleware_device_service
    release_middleware_device_modules
    release_middleware_protocol
}

function release_app_framework() {
  local app_framework_path="app/framework/pipeline"
  cp -rf ${TOP_DIR}/bazel-bin/${app_framework_path}/airos_app_framework ${OUT_BIN}
  cp -rf ${TOP_DIR}/${app_framework_path}/conf/* ${OUT_CONF}
}

function release_app_modules() {
    local APP_MIDULES=(demo v2x_application v2x_scenario)
    local app_modules_path="app/modules"
    for app in ${APP_MIDULES[@]}
    do
        local OUT_APP_LIB="${OUT_ROOT}/app/lib"
        local OUT_APP_CONF="${OUT_ROOT}/app/conf"
        local so_path=${OUT_APP_LIB}/airos_${app}
        local conf_path=${OUT_APP_CONF}/airos_${app}
        [ -d "${so_path}" ] || mkdir -p ${so_path}
        [ -d "${conf_path}" ] || mkdir -p ${conf_path}
        copy_lib_from_dir ${TOP_DIR}/bazel-bin/${app_modules_path}/${app} ${so_path}
        cp -f ${TOP_DIR}/${app_modules_path}/${app}/conf/* ${conf_path}
        cp -f ${TOP_DIR}/${app_modules_path}/${app}/conf/app_lib_cfg.pb ${so_path}
    done
}

function release_appliaction() {
    release_app_framework
    release_app_modules
}

function release_middleware_device_modules() {
    for device in ${SUPPORTED_DEVICE_SERVICE[@]}
    do
        local OUT_DEVICES_LIB="${OUT_ROOT}/device/lib"
        local so_path=${OUT_DEVICES_LIB}/${device}/airos_${device}
        [ -d "${so_path}" ] || mkdir -p ${so_path}
        cp -f ${TOP_DIR}/bazel-bin/${device_modules_path}/${device}/lib*.so ${so_path}
        cp -f ${TOP_DIR}/${device_modules_path}/${device}/conf/* ${so_path}
    done
}

function init_out_path() {
  local out_dirs=(
    "${OUT_ROOT}" 
    "${OUT_DAG}" 
    "${OUT_CONF}" 
    "${OUT_LIB}" 
    "${OUT_BIN}" 
  )
  mkdir -p "${out_dirs[@]}"
}

function build_all() {
    #    bazel build -- //... -//package/...
    PYTHON_BIN=$(which python3)
    bazel build //air_service/... //app/... //base/... //middleware/... //third_party/...  \
          //tools/... \
          --@rules_cuda//cuda:enable=True \
          --cxxopt="-DENABLE_ENCRYPTION=0" \
          --python_version=PY3 \
          --action_env=PYTHON_BIN_PATH=$PYTHON_BIN \
          --strategy=PythonCompile=standalone

    echo "build all done!"
}

function install_all() {
    # 先删除目录，防止文件替换不了
    rm -rf ${OUT_ROOT}
    del_out_dir
    init_out_path
    release_common_param
    release_services
    release_middleware
    release_appliaction
    cp -rf setup.bash ${OUT_ROOT}
    echo "install all done!"
}

function release_out {
    local out_dir=${OUT_3RD}
    [ -d "${out_dir}" ] || mkdir -p ${out_dir}

    for lib in $(ls /opt)
    do
        if [ -e "/opt/${lib}/lib" ]; then
            find /opt/${lib}/lib/ -name "lib*.so*" | grep -v stubs | xargs -I {} cp -f {} ${out_dir}/
        fi
    done

    local bazel_cache_lib_path="bazel-bin/_solib_*/"

    find ${bazel_cache_lib_path} -name "lib*.so*" | xargs -I {} cp -f {} ${out_dir}/
    echo "release done."
}

function clean_func() {
    bazel clean
    if [ -d $OUT_ROOT ]; then
        rm -rf $OUT_ROOT
    fi
    echo "clean done!"
}

function run_ut() {
    pushd ${TOP_DIR} > /dev/null
        source setup.bash
    popd

    run_base_device_connect_ut

    echo "ut done!"
}

function main() {
    if [ "$#" -eq 0 ]; then # $#参数个数为0，退出
        exit 0;
    fi

    env_setup

    local cmd="$1"
    shift
    case "${cmd}" in
        build)
            build_all
            install_all
            ;;
        test)
            build_all
            install_all
            release_out
            run_ut
            ;;
        clean)
            clean_func
            ;;
        release)
            build_all
            install_all
            release_out
            generate_version_file
            ;;
        pack)
            build_all
            install_all
            release_out
            source "setup.bash"
            tar_package
            ;;
        *)
            ;;
    esac
}

main "$@" # 传入$@所有参数，执行main