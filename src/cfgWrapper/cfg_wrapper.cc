// Copyright(c) 2022-present, Salvatore Cuzzilla (Swisscom AG)
// Distributed under the MIT License (http://opensource.org/licenses/MIT)


// mdt-dialout-collector Library headers
#include "cfg_wrapper.h"


bool CfgWrapper::BuildCfgWrapper(
    const std::string &writer_id)
{
    set_writer_id(writer_id);

    return true;
}

void CfgWrapper::DisplayCfgWrapper()
{
    const std::string writer_id = get_writer_id();

    std::cout << writer_id << "\n";
}

