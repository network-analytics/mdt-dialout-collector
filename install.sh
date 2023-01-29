#!/bin/sh

# Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
# Distributed under the MIT License (http://opensource.org/licenses/MIT)


set -o errexit
set -o nounset
#set -o pipefail

grpc_dev_min_version=145
libjsoncpp_dev_min_version=184
librdkafka_dev_min_version=160
libconfig_dev_min_version=150
libspdlog_dev_min_version=150
libzmq3_dev_min_version=432

sysdeb_install_list="libssl-dev libfmt-dev"

# gRPC install parameters
readonly grpc_url="https://github.com/grpc/grpc"
readonly grpc_version="v1.45.2"
readonly grpc_clone_dir="$HOME/grpc"
readonly grpc_install_dir="$HOME/.local"

# MDT install parameters
readonly mdt_url="https://github.com/network-analytics/mdt-dialout-collector.git"
readonly mdt_version="v1.1.1"
#readonly mdt_version="main"
readonly mdt_install_dir="/opt/mdt-dialout-collector"

# librdkafka parameters
readonly librdkafka_url="https://github.com/edenhill/librdkafka.git"
readonly librdkafka_version="v1.6.0"
readonly librdkafka_clone_dir="/opt/librdkafka"

# libjsoncpp parameters
readonly libjsoncpp_url="https://github.com/open-source-parsers/jsoncpp"
readonly libjsoncpp_version="1.8.4"
readonly libjsoncpp_clone_dir="/opt/libjsoncpp"

work_dir="$(dirname "$(readlink --canonicalize-existing "${0}" 2> /dev/null)")"

readonly epoch=$(date +'%s')
readonly available_vcpu=$(egrep 'processor' /proc/cpuinfo | wc -l)
readonly err_vcpu_failure=67
readonly err_grpc_ver_fail=68
readonly err_libjsoncpp_ver_fail=69
readonly err_librdkafka_ver_fail=70
readonly err_yum_failure=71
readonly err_apt_failure=72
readonly err_run_as_root=73
readonly err_git_clone_failure=74
readonly err_ds_release_undetected=75
readonly err_ds_undetected=76
readonly err_os_undetected=77
readonly err_cmd_notfound=78
readonly err_validating_input=79
readonly err_reading_file=80
readonly err_parsing_options=81
readonly err_missing_options=82
readonly err_unknown_options=83
readonly err_missing_options_arg=84
readonly err_unimplemented_options=85
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
      die "error - mind your options/arguments - [ -h ] to know more" "${err_unknown_options}"
      ;;
    \?)
      die "error - mind your options/arguments - [ -h ] to know more" "${err_missing_options_arg}"
      ;;
    *)
      die "error - mind your options/arguments - [ -h ] to know more" "${err_unimplemented_options}"
      ;;
    esac
  done
}
shift $((OPTIND -1))

clean_up() {
  #echo "clean_up()"
  #if [ -d ${grpc_clone_dir} ]; then
  #  rm -rf ${grpc_clone_dir}
  #fi

  if [ -d "${librdkafka_clone_dir}" ]; then
    rm -rf "${librdkafka_clone_dir}"
  fi

  if [ -d "${libjsoncpp_clone_dir}" ]; then
    rm -rf "${libjsoncpp_clone_dir}"
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
      die "error - undetected operating system" "${err_os_undetected}"
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
      die "error - undetected linux distribution" "${err_ds_undetected}"
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
        die "error - os_release_detect()" "${err_unimplemented_options}"
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
    die "error - undetected os release" "${err_ds_release_undetected}"
  fi
}

check_if_root() {
  local id_chk="$(id -u)"
  if [ ! "${id_chk}" -eq 0 ]; then
    die "error - run as root" "${err_run_as_root}"
  fi
}

detect_sysdeb_lib() {
  local lib="${1}"
  libjsoncpp_dev=0   # min 184
  librdkafka_dev=0   # min 160
  libconfig_dev=0    # min 150
  libspdlog_dev=0    # min 150
  libzmq3_dev=0      # min 434
  #libssl_dev=0      #
  #libfmt_dev=0      #
  grpc_dev=0         # TBD

  local apt_cache_show=1
  local lib_generic_version=$(apt-cache show ${lib}   | \
                              egrep Version           | \
                              awk -F ":" '{print $2}' | \
                              tr -d "\" \"|\.|\-"     | \
                              cut -c1-3)
  local lib_spdlog_version=$(apt-cache show libspdlog-dev | \
                             egrep Version                | \
                             awk -F " " '{print $2}'      | \
                             tr -d "\" \"|\.|\-"          | \
                             cut -c1-5                    | \
                             awk -F ":" '{print $2}')
  apt_cache_show="$?"

  if [ ! "${apt_cache_show}" -eq 0 ]; then
    die "error - apt-cache show failure" "${err_apt_failure}"
  fi

  if [ "${lib}" = "libjsoncpp-dev" ]; then
    if [ "${lib_generic_version}" -ge "${libjsoncpp_dev_min_version}" ]; then
      sysdeb_install_list="${sysdeb_install_list} ${lib}"
      libjsoncpp_dev=1
    fi
  fi

  if [ "${lib}" = "librdkafka-dev" ]; then
    if [ "${lib_generic_version}" -ge "${librdkafka_dev_min_version}" ]; then
      sysdeb_install_list="${sysdeb_install_list} ${lib}"
      librdkafka_dev=1
    fi
  fi

  if [ "${lib}" = "libconfig++-dev" ]; then
    if [ "${lib_generic_version}" -ge "${libconfig_dev_min_version}" ]; then
      sysdeb_install_list="${sysdeb_install_list} ${lib}"
      libconfig_dev=1
    fi
  fi

  if [ "${lib}" = "libspdlog-dev" ]; then
    if [ "${lib_spdlog_version}" -ge "${libspdlog_dev_min_version}" ]; then
      sysdeb_install_list="${sysdeb_install_list} ${lib}"
      libspdlog_dev=1
    fi
  fi

  if [ "${lib}" = "libzmq3-dev" ]; then
    if [ "${lib_generic_version}" -ge "${libzmq3_dev_min_version}" ]; then
      sysdeb_install_list="${sysdeb_install_list} ${lib}"
      libzmq3_dev=1
    fi
  fi
}

# TBD
# Additional development required
detect_sysrpm_lib() {
  #local lib="${1}"
  libjsoncpp_dev=0   # min 184
  librdkafka_dev=0   # min 160
  libconfig_dev=0    # min 150
  libspdlog_dev=0    # min 150
  libzmq3_dev=0      # min 434
  #libssl_dev=0      #
  #libfmt_dev=0      #
  grpc_dev=0
}

detect_installed_grpc() {
  grpc_installed=0
  local grpc_pkg_find=$(find /usr -name "grpc\+\+.pc")
  if [ ! -z "${grpc_pkg_find}" ]; then
    grpc_installed=1
  fi
  if [ "${grpc_installed}" -eq 1 ]; then
    grpc_raw_version=$(egrep "Version" "${grpc_pkg_find}" | awk -F ":" '{print $2}' | tr -d "")
    grpc_installed_version=$(egrep "Version" "${grpc_pkg_find}" | awk -F ":" '{print $2}' | tr -d "\" \"|\." | cut -c1-3)
  fi
}

detect_installed_librdkafka() {
  rdkafka_installed=0
  local rdkafka_pkg_find=$(find /usr -name "rdkafka\+\+.pc")
  if [ ! -z "${rdkafka_pkg_find}" ]; then
    rdkafka_installed=1
  fi
  if [ "${rdkafka_installed}" -eq 1 ]; then
    rdkafka_raw_version=$(egrep "Version" "${rdkafka_pkg_find}" | awk -F ":" '{print $2}' | tr -d "")
    rdkafka_version=$(egrep "Version" "${rdkafka_pkg_find}" | awk -F ":" '{print $2}' | tr -d "\" \"|\.")
    rdkafka_pkg_find_root=$(echo "${rdkafka_pkg_find}" | sed 's/\/rdkafka++.pc$//g')
  fi
}

librdkafka_install_from_src() {
  # librdkafka not installed ---> then install
  if [ "${rdkafka_installed}" -eq 0 ]; then
    local git_clone_rdkafka=1
    if [ ! -d "${librdkafka_clone_dir}" ]; then
      git clone -b "${librdkafka_version}" "${librdkafka_url}" "${librdkafka_clone_dir}"
      git_clone_rdkafka="$?"
    else
      # assuming that the clone was already performed
      git_clone_rdkafka=0
    fi
    cd "${librdkafka_clone_dir}"
    ./configure
    if [ "${available_vcpu}" -le 1 ]; then
        die "error - requires vcpu > 1)" "${err_vcpu_failure}"
    else
      make -j`echo $((${available_vcpu} - 1))`
      make install
    fi
  else
    # inform about the installed *.pc
    export PKG_CONFIG_PATH="$rdkafka_pkg_find_root"
  fi
}

detect_installed_libjsoncpp() {
  jsoncpp_installed=0
  local jsoncpp_pkg_find=$(find /usr -name "jsoncpp.pc")
  if [ ! -z "${jsoncpp_pkg_find}" ]; then
    jsoncpp_installed=1
  fi
  if [ "${jsoncpp_installed}" -eq 1 ]; then
    jsoncpp_raw_version=$(egrep "Version" "${jsoncpp_pkg_find}" | awk -F ":" '{print $2}' | tr -d "")
    jsoncpp_version=$(egrep "Version" "${jsoncpp_pkg_find}" | awk -F ":" '{print $2}' | tr -d "\" \"|\.")
    jsoncpp_pkg_find_root=$(echo "${jsoncpp_pkg_find}" | sed 's/\/jsoncpp.pc$//g')
  fi
}

libjsoncpp_install_from_src() {
  # libjsoncpp not installed ---> then install
  if [ "${jsoncpp_installed}" -eq 0 ]; then
    local git_clone_jsoncpp=1
    if [ ! -d "${libjsoncpp_clone_dir}" ]; then
      git clone -b "${libjsoncpp_version}" "${libjsoncpp_url}" "${libjsoncpp_clone_dir}"
      git_clone_jsoncpp="$?"
    else
      # assuming that the clone was already performed
      git_clone_jsoncpp=0
    fi
    mkdir -p "${libjsoncpp_clone_dir}/build"
    cd "${libjsoncpp_clone_dir}/build"
    cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON ../
    if [ "${available_vcpu}" -le 1 ]; then
        die "error - requires vcpu > 1)" "${err_vcpu_failure}"
    else
      make -j`echo $((${available_vcpu} - 1))`
      make install
    fi
  else
    export PKG_CONFIG_PATH="$jsoncpp_pkg_find_root"
  fi
}

grpc_framework_install() {
  export PATH="$PATH:${grpc_install_dir}/bin"

  if [ ! -d "${grpc_install_dir}" ]; then
    mkdir -p "${grpc_install_dir}"
  fi

  local git_clone=1
  if [ ! -d "${grpc_clone_dir}" ]; then
    git clone --recurse-submodules -b "${grpc_version}" --depth 1 \
      --shallow-submodules "${grpc_url}" "${grpc_clone_dir}"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${err_git_clone_failure}"
  fi

  cd "${grpc_clone_dir}"
  if [ ! -d "cmake/build" ]; then
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

    if [ "${available_vcpu}" -le 1 ]; then
        die "error - requires vcpu > 1)" "${err_vcpu_failure}"
    else
      make -j`echo $((${available_vcpu} - 1))`
      make install
    fi
  fi
}

grpc_collector_bin_install_deb() {
  #echo "grpc_collector_bin_install_deb()"
  local apt_install=1
  apt-get install -y $(echo "${sysdeb_install_list}")
  apt_install="$?"

  if [ ! "${apt_install}" -eq 0 ]; then
    die "error - apt install failure" "${err_apt_failure}"
  fi

  if [ "${librdkafka_dev}" -eq 0 ]; then
    detect_installed_librdkafka
  fi

  if [ "${rdkafka_installed}" -eq 1 ] && [ "${rdkafka_version}" -lt "${librdkafka_dev_min_version}" ]; then
    die "error - the installed version of librdkafka (${rdkafka_raw_version}) is too old" "${err_librdkafka_ver_fail}"
  else
    librdkafka_install_from_src
  fi

  if [ "${libjsoncpp_dev}" -eq 0 ]; then
    detect_installed_libjsoncpp
  fi

  if [ "${jsoncpp_installed}" -eq 1 ] && [ "${jsoncpp_version}" -lt "${libjsoncpp_dev_min_version}" ]; then
    die "error - the installed version of libjsoncpp (${jsoncpp_raw_version}) is too old" "${err_libjsoncpp_ver_fail}"
  else
    libjsoncpp_install_from_src
  fi

  local git_clone=1
  if [ ! -d "${mdt_install_dir}" ]; then
    git clone "${mdt_url}" -b "${mdt_version}" "${mdt_install_dir}"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${err_git_clone_failure}"
  fi

  if [ ! -d "${mdt_install_dir}/build" ]; then
    mkdir -p "${mdt_install_dir}/build"
  fi

  cd "${mdt_install_dir}/build"
  cmake ../
  if [ "${available_vcpu}" -le 1 ]; then
      die "error - requires vcpu > 1)" "${err_vcpu_failure}"
  else
    make -j`echo $((${available_vcpu} - 1))`
  fi

  # deploy a customizable configuration
}

grpc_collector_bin_install_rpm() {
  #echo "grpc_collector_bin_install_rpm()"
  local yum_install=1
  yum install -y jsoncpp-devel libconfig-devel spdlog-devel cppzmq-devel openssl-devel
  yum_install="$?"

  sed -i '/SPDLOG_FMT_EXTERNAL/s/^\/\/ //g' /usr/include/spdlog/tweakme.h

  if [ ! "${yum_install}" -eq 0 ]; then
    die "error - yum install failure" "${err_yum_failure}"
  fi

  if [ "${librdkafka_dev}" -eq 0 ]; then
    detect_installed_librdkafka
  fi

  # librdkafka installed && version number insuffcient ---> exit!
  if [ "${rdkafka_installed}" -eq 1 ] && [ "${rdkafka_version}" -lt "${librdkafka_dev_min_version}" ]; then
    die "error - the installed version of librdkafka (${rdkafka_raw_version}) is too old" "${err_librdkafka_ver_fail}"
  else
    librdkafka_install_from_src
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
    die "error - git clone failure" "${err_git_clone_failure}"
  fi

  if [ ! -d "${mdt_install_dir}/build" ]; then
    mkdir -p "${mdt_install_dir}/build"
  fi

  cd "${mdt_install_dir}/build"
  cmake ../
  if [ "${available_vcpu}" -le 1 ]; then
      die "error - requires vcpu > 1)" "${err_vcpu_failure}"
  else
    make -j`echo $((${available_vcpu} - 1))`
  fi

  # deploy a customizable configuration
}

grpc_collector_lib_install_deb() {
  #echo "grpc_collector_lib_install_deb()"

  if [ ! -f "/usr/local/bin/grpc_cpp_plugin" ]; then
    ln -s "${grpc_install_dir}/bin/grpc_cpp_plugin" "/usr/local/bin/grpc_cpp_plugin"
  fi

  local apt_install=1
  apt-get install -y $(echo "${sysdeb_install_list}")
  apt_install="$?"

  if [ ! "${apt_install}" -eq 0 ]; then
    die "error - apt install failure" "${err_apt_failure}"
  fi

  if [ "${librdkafka_dev}" -eq 0 ]; then
    detect_installed_librdkafka
  fi

  if [ "${rdkafka_installed}" -eq 1 ] && [ "${rdkafka_version}" -lt "${librdkafka_dev_min_version}" ]; then
    die "error - the installed version of librdkafka (${rdkafka_raw_version}) is too old" "${err_librdkafka_ver_fail}"
  else
    librdkafka_install_from_src
  fi

  if [ "${libjsoncpp_dev}" -eq 0 ]; then
    detect_installed_libjsoncpp
  fi

  if [ "${jsoncpp_installed}" -eq 1 ] && [ "${jsoncpp_version}" -lt "${libjsoncpp_dev_min_version}" ]; then
    die "error - the installed version of libjsoncpp (${jsoncpp_raw_version}) is too old" "${err_libjsoncpp_ver_fail}"
  else
    libjsoncpp_install_from_src
  fi

  local git_clone=1
  if [ ! -d "${mdt_install_dir}" ]; then
    git clone "${mdt_url}" -b "${mdt_version}" "${mdt_install_dir}"
    git_clone="$?"
  else
    # assuming that the clone was already performed
    git_clone=0
  fi

  if [ ! "${git_clone}" -eq 0 ]; then
    die "error - git clone failure" "${err_git_clone_failure}"
  fi

  cd "${mdt_install_dir}"
  ./autogen.sh
  PKG_CONFIG_PATH="$grpc_install_dir/lib/pkgconfig:/usr/local/lib/pkgconfig" \
    CPPFLAGS="-I${grpc_install_dir}/include -I/usr/include/jsoncpp" ./configure
  if [ "${available_vcpu}" -le 1 ]; then
      die "error - requires vcpu > 1)" "${err_vcpu_failure}"
  else
    make -j`echo $((${available_vcpu} - 1))`
    make install
  fi

  # deploy a customizable configuration
}

grpc_collector_lib_install_rpm() {
  #echo "grpc_collector_lib_install_rpm()"

  if [ ! -f "/usr/local/bin/grpc_cpp_plugin" ]; then
    ln -s "${grpc_install_dir}/bin/grpc_cpp_plugin" "/usr/local/bin/grpc_cpp_plugin"
  fi

  local yum_install=1
  yum install -y jsoncpp-devel libconfig-devel spdlog-devel cppzmq-devel openssl-devel
  yum_install="$?"

  sed -i '/SPDLOG_FMT_EXTERNAL/s/^\/\/ //g' /usr/include/spdlog/tweakme.h

  if [ ! "${yum_install}" -eq 0 ]; then
    die "error - yum install failure" "${err_yum_failure}"
  fi

  if [ "${librdkafka_dev}" -eq 0 ]; then
    detect_installed_librdkafka
  fi

  # librdkafka installed && version number insuffcient ---> exit!
  if [ "${rdkafka_installed}" -eq 1 ] && [ "${rdkafka_version}" -lt "${librdkafka_dev_min_version}" ]; then
    die "error - the installed version of librdkafka (${rdkafka_raw_version}) is too old" "${err_librdkafka_ver_fail}"
  else
    librdkafka_install_from_src
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
    die "error - git clone failure" "${err_git_clone_failure}"
  fi

  cd "${mdt_install_dir}"
  ./autogen.sh
  PKG_CONFIG_PATH="$grpc_install_dir/lib/pkgconfig:$grpc_install_dir/lib64/pkgconfig:/usr/local/lib/pkgconfig" \
    CPPFLAGS="-I${grpc_install_dir}/include" ./configure
  if [ "${available_vcpu}" -le 1 ]; then
      die "error - requires vcpu > 1)" "${err_vcpu_failure}"
  else
    make -j`echo $((${available_vcpu} - 1))`
    make install
  fi

  # deploy a customizable configuration
}

grpc_collector_deploy() {
  _os_info="$(os_release_detect "release")"

  command -v uname      >/dev/null 2>&1 || die "error - expected uname command"                 "${err_cmd_notfound}"
  command -v egrep      >/dev/null 2>&1 || die "error - expected egrep command"                 "${err_cmd_notfound}"
  command -v awk        >/dev/null 2>&1 || die "error - expected awk command"                   "${err_cmd_notfound}"
  command -v git        >/dev/null 2>&1 || die "error - expected git command"                   "${err_cmd_notfound}"
  command -v id         >/dev/null 2>&1 || die "error - expected id command"                    "${err_cmd_notfound}"
  command -v make       >/dev/null 2>&1 || die "error - expected make command"                  "${err_cmd_notfound}"
  command -v cmake      >/dev/null 2>&1 || die "error - expected cmake command"                 "${err_cmd_notfound}"
  command -v gcc        >/dev/null 2>&1 || die "error - expected gcc command"                   "${err_cmd_notfound}"
  command -v autoreconf >/dev/null 2>&1 || die "error - expected autoreconf (autoconf) command" "${err_cmd_notfound}"
  command -v libtoolize >/dev/null 2>&1 || die "error - expected libtoolize (libtool) command"  "${err_cmd_notfound}"
  command -v cut        >/dev/null 2>&1 || die "error - expected cut command"                   "${err_cmd_notfound}"

  case "${_os_info}" in
  "Linux debian 11"    | \
  "Linux ubuntu 20.04" | \
  "Linux ubuntu 22.04" | \
  "Linux ubuntu 22.10" | \
  "Linux pop 22.04")
    #echo "grpc_collector_deploy_deb()"
    command -v apt-get >/dev/null 2>&1 || die "error - expected apt command" "${err_cmd_notfound}"
    command -v g++     >/dev/null 2>&1 || die "error - expected g++ command" "${err_cmd_notfound}"
    check_if_root
    detect_sysdeb_lib "libjsoncpp-dev"
    detect_sysdeb_lib "librdkafka-dev"
    detect_sysdeb_lib "libconfig++-dev"
    detect_sysdeb_lib "libspdlog-dev"
    detect_sysdeb_lib "libzmq3-dev"
    if [ "${grpc_dev}" -eq 0 ]; then
      detect_installed_grpc
    fi
    if [ "${grpc_installed}" -eq 1 ] && [ "${grpc_installed_version}" -lt "${grpc_dev_min_version}" ]; then
      die "error - the installed version of gRPC (${grpc_raw_version}) is too old" "${err_grpc_ver_fail}"
    else
      grpc_framework_install
    fi
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
    command -v yum     >/dev/null 2>&1 || die "error - expected yum command" "${err_cmd_notfound}"
    check_if_root
    # Switch to a recent gcc version
    source /opt/rh/gcc-toolset-11/enable
    detect_sysrpm_lib
    if [ "${grpc_dev}" -eq 0 ]; then
      detect_installed_grpc
    fi
    if [ "${grpc_installed}" -eq 1 ] && [ "${grpc_installed_version}" -lt "${grpc_dev_min_version}" ]; then
      die "error - the installed version of gRPC (${grpc_raw_version}) is too old" "${err_grpc_ver_fail}"
    else
      grpc_framework_install
    fi
    if [ "${b_option_flag}" -eq 1 ]; then
      grpc_collector_bin_install_rpm
    fi
    if [ "${l_option_flag}" -eq 1 ]; then
      grpc_collector_lib_install_rpm
    fi
    ;;
  *)
    # Should never see the sun
    die "error - grpc_collector_install(): ${_os_info} offcially not supported" "${err_unimplemented_options}"
    ;;
  esac
}

parse_user_options "${@}"

if [ "${h_option_flag}" -eq 1 ]; then
  usage
  exit 0
fi

# both bin & lib to 1: invalid
if [ "${b_option_flag}" -eq 1 ] && [ "${l_option_flag}" -eq 1 ]; then
  die "error - mind  your options/arguments - [ -h ] to know more" "${err_validating_input}"
fi

# both bin & lib to 0: invalid
if [ "${b_option_flag}" -eq 0 ] && [ "${l_option_flag}" -eq 0 ]; then
  die "error - mind  your options/arguments - [ -h ] to know more" "${err_validating_input}"
fi

if [ "${b_option_flag}" -eq 1 ] || [ "${l_option_flag}" -eq 1 ]; then
  grpc_collector_deploy
fi

exit 0

