CUR_DIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd -P)"

pushd "${CUR_DIR}/../../" > /dev/null
    source setup.bash
popd > /dev/null

./ins_test_tool conf/ins1.yaml 