#!/bin/sh

# Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)


set -o errexit
set -o nounset
#set -o pipefail

# gRPC install parameters
readonly grpc_url="https://github.com/grpc/grpc"
readonly grpc_version="v1.45.2"
readonly grpc_install_dir="$HOME/.local"

# MDT install parameters
readonly mdt_url="https://github.com/scuzzilla/mdt-dialout-collector.git"
readonly mdt_install_dir="/opt/mdt-dialout-collector"

# librdkafka parameters
readonly librdkafka_url="https://github.com/edenhill/librdkafka.git"
readonly librdkafka_install_dir="/opt/librdkafka"

work_dir="$(dirname "$(readlink --canonicalize-existing "${0}" 2> /dev/null)")"

readonly epoch=$(date +'%s')
readonly error_yum_install_failure=71
readonly error_apt_install_failure=72
readonly error_run_as_root=73
readonly error_git_clone_failure=74
readonly error_ds_release_undetected=75
readonly error_ds_undetected=76
readonly error_os_undetected=77
readonly error_cmd_notfound=78
readonly error_validating_input=79
readonly error_reading_file=80
readonly error_parsing_options=81
readonly error_missing_options=82
readonly error_unknown_options=83
readonly error_missing_options_arg=84
readonly error_unimplemented_options=85
readonly script_name="${0##*/}"

h_option_flag=0
b_option_flag=0
l_option_flag=0

trap clean_up EXIT INT HUP

usage() {
  cat <<EOF
  Usage: ${script_name} [-b] || [-l] || [-h]

  DESCRIPTION:
    The install.sh script can be used to automate the install procces of the mdt-dialout-collector.
    Both the binary & the library version are supported.

  OPTIONS:
    -h
      Print this help and exit.
    -b
      Install the standalone flavor of the mdt-dialout-collector (binary version).
    -l
      Install the library flavor of the mdt-dialout-collector (can be used with pmacct, library version).
EOF
}

parse_user_options() {
  while getopts "blh" opts; do
    case "${opts}" in
    b)
      b_option_flag=1
      ;;
    l)
      l_option_flag=1
      ;;
    h)
      h_option_flag=1
      ;;
    :)
      die "error - mind your options/arguments - [ -h ] to know more" "${error_unknown_options}"
      ;;
    \?)
      die "error - mind your options/arguments - [ -h ] to know more" "${error_missing_options_arg}"
      ;;
    *)
      die "error - mind your options/arguments - [ -h ] to know more" "${error_unimplemented_options}"
      ;;
    esac
  done
}
shift $((OPTIND -1))

clean_up() {
  echo "clean_up()"
  if [ -d $HOME/grpc ]; then
    rm -rf $HOME/grpc
  fi

  if [ -d "${librdkafka_install_dir}" ]; then
    rm -rf "${librdkafka_install_dir}"
  fi
}

die() {
  local readonly msg="${1}"
  local readonly code="${2:-90}"
  echo "${msg}" >&2
  exit "${code}"
}

os_release_detect() {
  local detect="${1}"
  local item_found_flag=0;

  case "${detect}" in
  os)
    # Supported os
    set -- BSD Linux
    local operating_system="$(uname)"

    for item in "$@";
    do
      if [ "${item}" = "${operating_system}" ]; then
        item_found_flag=1
        break
      fi
    done

    if [ "${item_found_flag}" -eq 1 ]; then
      echo "${operating_system}";
    else
      die "error - undetected operating system" "${error_os_undetected}"
    fi
    ;;
  flavor)
    # Supported distro
    set -- debian ubuntu pop centos OpenBSD
    local linux_distribution="$(egrep '^ID=' /etc/os-release | awk -F "=" '{print $2}' | tr -d "\"")"
    local bsd_distribution="none"

    for item in "$@";
    do
      if [ "${item}" = "${linux_distribution}" ]; then
        item_found_flag=1
        break
      fi
    done

    if [ "${item_found_flag}" -eq 1 ]; then
      echo "${linux_distribution}";
    else
      die "error - undetected linux distribution" "${error_ds_undetected}"
    fi
    ;;
  release)
    _os=$(os_release_detect "os")
    _flavor=$(os_release_detect "flavor")

    if [ "${_os}" = "Linux" ]; then
      local distro_release="$(egrep '^VERSION_ID=' /etc/os-release | awk -F "=" '{print $2}' | tr -d "\"")"

      case "${_flavor}" in
      debian)
        # Supported debian releases
        set -- 11

        for item in "$@";
        do
          if [ "${item}" = "${distro_release}" ]; then
            item_found_flag=1
            break
          fi
        done

        os_release_helper
        ;;
      ubuntu|pop)
        # Supported ubuntu/pop releases
        set -- 20.04 22.04 22.10

        for item in "$@";
        do
          if [ "${item}" = "${distro_release}" ]; then
            item_found_flag=1
            break
          fi
        done

        os_release_helper
        ;;
      centos|redhat)
        # Supported centos releases
        set -- 8 9

        for item in "$@";
        do
          if [ "${item}" = "${distro_release}" ]; then
            item_found_flag=1
            break
          fi
        done

        os_release_helper
        ;;
      *)
        # Should never see the sun
        die "error - os_release_detect()" "${error_unimplemented_options}"
        ;;
      esac
    fi
    ;;
  esac
}

os_release_helper() {
  if [ "${item_found_flag}" -eq 1 ]; then
    echo "${_os}" "${_flavor}" "${distro_release}";
  else
    die "error - undetected os release" "${error_ds_release_undetected}"
  fi
}

check_if_root() {
	local id_chk="$(id -u)"
  if [ ! "${id_chk}" -eq 0 ]; then
    die "error - run as root" "${error_run_as_root}"
  fi
}

grpc_framework_install() {
  export grpc_install_dir
  export PATH="${grpc_install_dir}/bin:$PATH"

  if [ ! -d "${grpc_install_dir}" ]; then
    mkdir -p "${grpc_install_dir}"
  fi

  local git_clone=1
  if [ ! -d "$HOME/grpc" ]; then
    git clone --recurse-submodules -b "${grpc_version}" --depth 1 \
      --shallow-submodules "${grpc_url}" "$HOME/grpc"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${error_git_clone_failure}"
  fi

  cd "$HOME/grpc"
  if [ ! -d "cmake/install" ]; then
    mkdir -p cmake/build
    cd cmake/build
    cmake -DgRPC_INSTALL=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DgRPC_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX="${grpc_install_dir}" \
    -DABSL_PROPAGATE_CXX_STD=ON \
    -DgRPC_ABSL_PROVIDER=module \
    -DgRPC_CARES_PROVIDER=module \
    -DgRPC_PROTOBUF_PROVIDER=module \
    -DgRPC_RE2_PROVIDER=module \
    -DgRPC_SSL_PROVIDER=module \
    -DgRPC_ZLIB_PROVIDER=module \
    ../..

    make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`

    make install
  fi
}

grpc_collector_bin_install_deb() {
  #echo "grpc_collector_bin_install_deb()"
  local apt_install=1
  apt-get install -y libjsoncpp-dev librdkafka-dev libconfig++-dev libspdlog-dev libzmq3-dev libssl-dev libfmt-dev
  apt_install="$?"

  if [ ! "${apt_install}" -eq 0 ]; then
    die "error - apt install failure" "${error_apt_install_failure}"
  fi

  local git_clone=1
  if [ ! -d "${mdt_install_dir}" ]; then
    git clone "${mdt_url}" "${mdt_install_dir}"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${error_git_clone_failure}"
  fi

  cd "${mdt_install_dir}"
  if [ ! -d "build" ]; then
    mkdir -p build
    cd build
    cmake ../
    make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
  fi

  # deploy a customizable configuration
}

grpc_collector_bin_install_rpm() {
  #echo "grpc_collector_bin_install_rpm()"
  # Required by librdkafka
  export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

  local yum_install=1
  yum install -y jsoncpp-devel libconfig-devel spdlog-devel cppzmq-devel openssl-devel
  yum_install="$?"

  sed -i '/SPDLOG_FMT_EXTERNAL/s/^\/\/ //g' /usr/include/spdlog/tweakme.h

  if [ ! "${yum_install}" -eq 0 ]; then
    die "error - apt install failure" "${error_yum_install_failure}"
  fi

  local git_clone_rdkafka=1
  if [ ! -d "${librdkafka_install_dir}" ]; then
    git clone "${librdkafka_url}" "${librdkafka_install_dir}"
    git_clone_rdkafka="$?"
  else
    # assuming that the clone was already performed
    git_clone_rdkafka=0
  fi

  cd "${librdkafka_install_dir}"
  ./configure
  make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
  make install

  local git_clone=1
  if [ ! -d "${mdt_install_dir}" ]; then
    git clone "${mdt_url}" "${mdt_install_dir}"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${error_git_clone_failure}"
  fi

  cd "${mdt_install_dir}"
  if [ ! -d "build" ]; then
    mkdir -p build
    cd build
    cmake ../
    make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
  fi

  # deploy a customizable configuration
}

grpc_collector_lib_install_deb() {
  #echo "grpc_collector_lib_install_deb()"
  export PKG_CONFIG_PATH=$grpc_install_dir/lib/pkgconfig

  if [ ! -f "/usr/local/bin/grpc_cpp_plugin" ]; then
    ln -s "${grpc_install_dir}/bin/grpc_cpp_plugin" "/usr/local/bin/grpc_cpp_plugin"
  fi

  local apt_install=1
  apt-get install -y libjsoncpp-dev librdkafka-dev libconfig++-dev libspdlog-dev libzmq3-dev libssl-dev libfmt-dev
  apt_install="$?"

  if [ ! "${apt_install}" -eq 0 ]; then
    die "error - apt install failure" "${error_apt_install_failure}"
  fi

  local git_clone=1
  if [ ! -d "${mdt_install_dir}" ]; then
    git clone "${mdt_url}" "${mdt_install_dir}"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${error_git_clone_failure}"
  fi

  cd "${mdt_install_dir}"
  ./autogen.sh
  CPPFLAGS="-I${grpc_install_dir}/include -I/usr/include/jsoncpp" ./configure
  make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
  make install

  # deploy a customizable configuration
}

grpc_collector_lib_install_rpm() {
  #echo "grpc_collector_lib_install_rpm()"
  export PKG_CONFIG_PATH=$grpc_install_dir/lib/pkgconfig:$grpc_install_dir/lib64/pkgconfig:/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

  if [ ! -f "/usr/local/bin/grpc_cpp_plugin" ]; then
    ln -s "${grpc_install_dir}/bin/grpc_cpp_plugin" "/usr/local/bin/grpc_cpp_plugin"
  fi

  local yum_install=1
  yum install -y jsoncpp-devel libconfig-devel spdlog-devel cppzmq-devel openssl-devel
  yum_install="$?"

  sed -i '/SPDLOG_FMT_EXTERNAL/s/^\/\/ //g' /usr/include/spdlog/tweakme.h

  if [ ! "${yum_install}" -eq 0 ]; then
    die "error - apt install failure" "${error_yum_install_failure}"
  fi

  local git_clone_rdkafka=1
  if [ ! -d "${librdkafka_install_dir}" ]; then
    git clone "${librdkafka_url}" "${librdkafka_install_dir}"
    git_clone_rdkafka="$?"
  else
    # assuming that the clone was already performed
    git_clone_rdkafka=0
  fi

  cd "${librdkafka_install_dir}"
  ./configure
  make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
  make install

  local git_clone=1
  if [ ! -d "${mdt_install_dir}" ]; then
    git clone "${mdt_url}" "${mdt_install_dir}"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${error_git_clone_failure}"
  fi

  cd "${mdt_install_dir}"
  ./autogen.sh
  CPPFLAGS="-I${grpc_install_dir}/include" ./configure
  make -j`echo $(($(egrep 'processor' /proc/cpuinfo | wc -l) - 1))`
  make install

  # deploy a customizable configuration
}

grpc_collector_deploy() {
  _os_info="$(os_release_detect "release")"

  case "${_os_info}" in
  "Linux debian 11"    | \
  "Linux ubuntu 20.04" | \
  "Linux ubuntu 22.04" | \
  "Linux ubuntu 22.10" | \
  "Linux pop 22.04")
    #echo "grpc_collector_deploy_deb()"
    command -v apt-get >/dev/null 2>&1 || die "error - expected apt command"   "${error_cmd_notfound}"
    check_if_root
    grpc_framework_install
    if [ "${b_option_flag}" -eq 1 ]; then
      grpc_collector_bin_install_deb
    fi
    if [ "${l_option_flag}" -eq 1 ]; then
      grpc_collector_lib_install_deb
    fi
    ;;
  "Linux centos 8" | \
  "Linux centos 9" | \
  "Linux redhat 8" | \
  "Linux redhat 9")
    #echo "grpc_collector_deploy_rpm()"
    command -v yum     >/dev/null 2>&1 || die "error - expected yum command"   "${error_cmd_notfound}"
    check_if_root
    # Switch to a recent gcc version
    source /opt/rh/gcc-toolset-11/enable
    grpc_framework_install
    if [ "${b_option_flag}" -eq 1 ]; then
      grpc_collector_bin_install_rpm
    fi
    if [ "${l_option_flag}" -eq 1 ]; then
      grpc_collector_lib_install_rpm
    fi
    ;;
  *)
    # Should never see the sun
    die "error - grpc_collector_install()" "${error_unimplemented_options}"
    ;;
  esac
}

command -v uname      >/dev/null 2>&1 || die "error - expected uname command"                   "${error_cmd_notfound}"
command -v egrep      >/dev/null 2>&1 || die "error - expected egrep command"                   "${error_cmd_notfound}"
command -v awk        >/dev/null 2>&1 || die "error - expected awk command"                     "${error_cmd_notfound}"
command -v git        >/dev/null 2>&1 || die "error - expected git command"                     "${error_cmd_notfound}"
command -v id         >/dev/null 2>&1 || die "error - expected id command"                      "${error_cmd_notfound}"
command -v make       >/dev/null 2>&1 || die "error - expected make command"                    "${error_cmd_notfound}"
command -v cmake      >/dev/null 2>&1 || die "error - expected cmake command"                   "${error_cmd_notfound}"
command -v g++        >/dev/null 2>&1 || die "error - expected g++ command"                     "${error_cmd_notfound}"
command -v autoreconf >/dev/null 2>&1 || die "error - expected autoreconf (autoconf) command"   "${error_cmd_notfound}"

parse_user_options "${@}"

if [ "${h_option_flag}" -eq 1 ]; then
  usage
  exit 0
fi

# both bin & lib to 1: invalid
if [ "${b_option_flag}" -eq 1 ] && [ "${l_option_flag}" -eq 1 ]; then
  die "error - mind  your options/arguments - [ -h ] to know more" "${error_validating_input}"
fi

# both bin & lib to 0: invalid
if [ "${b_option_flag}" -eq 0 ] && [ "${l_option_flag}" -eq 0 ]; then
  die "error - mind  your options/arguments - [ -h ] to know more" "${error_validating_input}"
fi

if [ "${b_option_flag}" -eq 1 ] || [ "${l_option_flag}" -eq 1 ]; then
  grpc_collector_deploy
fi

exit 0

