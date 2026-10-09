/*
 * @Author: cs
 * @Date: 2024-02-01 10:16:47
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-01 14:26:47
 * @Description:
 */
#ifndef COMM_CONFIGURE_CONFIGERFILE_H_
#define COMM_CONFIGURE_CONFIGERFILE_H_

#include "base/common/network/json.h"
#include "base/common/network/file_util.h"
#include "base/common/network/string_util.h"
#include "base/common/network/define.h"

#include <fstream>
#include <type_traits>
#include <map>

using ConfigBlock = nlohmann::json;

NAMESPACE_AFL_BASE_START

class ConfigerFile final
{
public:
    explicit ConfigerFile(std::string fileName)
        : m_FileName(fileName)
        , m_ConfigBlocks(new ConfigBlock())
    {

        if(!afl::isFileExist(m_FileName.c_str()))
        {
            std::ofstream out(m_FileName);
            out << "{}";
        }

        std::ifstream in(m_FileName);
        try
        {
            in >> (*m_ConfigBlocks);
        }
        catch (ConfigBlock::exception& e)
        {
            std::cout << e.what() << std::endl;
            std::ofstream out(m_FileName);
            out << "{}";
//          abort();    /*!!!*/
        }
    }

    template<typename ...DIR>
    ConfigBlock& configBlock(DIR... dirs)
    {
        return getConfigBlock(*m_ConfigBlocks, dirs...);
    }

    const std::string& fileName() const
    {
        return m_FileName;
    }

    void reload()
    {
        m_ConfigBlocks->clear();

        if(!afl::isFileExist(m_FileName.c_str()))
        {
            std::ofstream out(m_FileName);
            out << "{}";
        }

        std::ifstream in(m_FileName);
        try
        {
            in >> (*m_ConfigBlocks);
        }
        catch (ConfigBlock::exception& e)
        {
            std::cout << e.what() << std::endl;
            std::ofstream out(m_FileName);
            out << "{}";
        }
    }

    void save()
    {
        std::ofstream out(m_FileName);
        out << m_ConfigBlocks->dump(4);
    }

private:
    template<typename Rest>
    ConfigBlock& getConfigBlock(ConfigBlock& cb, Rest r)
    {
        static_assert(std::is_same<decltype(r), std::string>::value, "Get configure Block with non-string type");
        return cb[r];
    }

    template<typename First, typename ...DIR>
    ConfigBlock& getConfigBlock(ConfigBlock& cb, First f, DIR ...dirs)
    {
        static_assert(std::is_same<decltype(f), std::string>::value, "Get configure Block with non-string type");
        return getConfigBlock(cb[f], dirs...);
    }

private:
    std::string m_FileName;
    std::unique_ptr<ConfigBlock> m_ConfigBlocks;
};

NAMESPACE_AFL_BASE_END

#endif /* COMM_CONFIGURE_CONFIGERFILE_H_ */
