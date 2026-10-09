/*
 * ConfigerManager.hpp
 *
 *  Created on: 2018年5月2日
 *      Author: cwy
 */
#ifndef BASE_CONFIGER_CONFIGERMANAGER_H_
#define BASE_CONFIGER_CONFIGERMANAGER_H_

#include "configer_data.h"
#include "base/common/network/file_util.h"

NAMESPACE_AFL_BASE_START
class ConfigerManager final
{
public:
    ConfigerManager(std::string dir = "./");
    ~ConfigerManager();

    void saveConfigure();
    void reloadConfigure();
    void reloadConfigure(std::string name);
    void unregisterConfiger(std::uint64_t hdl);

    std::shared_ptr<ConfigerFile> getConfigerFile(std::uint64_t hdl);

    template<typename T>
    std::uint64_t registerConfiger(std::string key, T& data, std::string file = std::string())
    {
        static_assert(std::is_base_of<ConfigerDataBase, T>::value, "configure data is not derive from ConfigerDataBase");

        if(file.empty())
        {
            file = m_DefaultConfigerFilePtr->fileName();
        }
        else
        {
            file = m_ConfigerDir + file;
        }

        if(m_Str2File.find(file) == m_Str2File.end())
        {
            afl::FileUtil::createRecursionDir(afl::FileUtil::dirName(file.c_str()).c_str());
            m_Str2File[file] = std::shared_ptr<ConfigerFile>(new ConfigerFile(file));
        }

        auto& cb = m_Str2File[file]->configBlock(key);

        if(cb.is_null())
        {
            try
            {
                cb = data;
            }
            catch(ConfigBlock::exception& e)
            {
                std::cout << "set default data to configure block error!" << e.what() << std::endl;
            }
        }
        else
        {
            try
            {
                data = cb;
            }
            catch(ConfigBlock::exception& e)
            {
                std::cout << "get configure data from configure block error!" << e.what() << std::endl;
            }
        }

        m_Str2File[file]->save();

        if(m_File2Module.find(m_Str2File[file]) == m_File2Module.end())
            m_File2Module[m_Str2File[file]] = std::map<ConfigerDataBase*, std::string>();

        ConfigerDataBase* cdb = &data;
        cdb->setFileName(file);
        m_File2Module[m_Str2File[file]][cdb] = key;

        return reinterpret_cast<std::uint64_t>(cdb);
    }

private:
    ConfigerManager(const ConfigerManager&) = delete;
    ConfigerManager& operator == (const ConfigerManager&) = delete;

    std::map<std::string, std::shared_ptr<ConfigerFile>> m_Str2File;
    std::map<std::shared_ptr<ConfigerFile>, std::map<ConfigerDataBase*, std::string>> m_File2Module;

    std::string m_ConfigerDir;
    std::shared_ptr<ConfigerFile> m_DefaultConfigerFilePtr;
};

NAMESPACE_AFL_BASE_END

#endif /* COMM_CONFIGURE_CONFIGERMANAGER_H_ */
