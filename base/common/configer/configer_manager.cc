/*
 * @Author: cs
 * @Date: 2024-02-01 9:06:47
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-01 13:20:47
 * @Description:
 */
#include "configer_manager.h"

NAMESPACE_AFL_BASE_START
ConfigerManager::ConfigerManager(std::string dir) : m_ConfigerDir(dir + "/")
{
//    std::string file = m_ConfigerDir + "default.cfg";
//    afl::FileUtil::createRecursionDir(afl::FileUtil::dirName(file.c_str()).c_str());
//    m_DefaultConfigerFilePtr.reset(new ConfigerFile(file));
//    m_Str2File[m_DefaultConfigerFilePtr->fileName()] = m_DefaultConfigerFilePtr;
}

ConfigerManager::~ConfigerManager()
{
    m_Str2File.clear();
    m_File2Module.clear();
}

std::shared_ptr<ConfigerFile> ConfigerManager::getConfigerFile(std::uint64_t hdl)
{
    ConfigerDataBase* cdb = reinterpret_cast<ConfigerDataBase*>(hdl);

    auto& file = cdb->getFileName();
    if(m_Str2File.find(file) == m_Str2File.end())
    {
        fprintf(stderr, "Could not find configure file name %s\n",  file.c_str());
        return nullptr;
    }

    return m_Str2File[file];
}

void ConfigerManager::unregisterConfiger(std::uint64_t hdl)
{
    ConfigerDataBase* cdb = reinterpret_cast<ConfigerDataBase*>(hdl);

    auto& file = cdb->getFileName();
    if(m_Str2File.find(file) == m_Str2File.end())
    {
        fprintf(stderr, "Could not find configure file name %s\n",  file.c_str());
        return;
    }

    auto configFile = m_Str2File[file];
    if(m_File2Module.find(configFile) == m_File2Module.end())
    {
        fprintf(stderr, "Could not find configure file shared pointer %s\n", file.c_str());
        return;
    }

    auto iter = m_File2Module[configFile].find( cdb );
    if(iter ==  m_File2Module[configFile].end())
    {
        fprintf(stderr, "Could not find configure Data pointer %p, file name %s\n", cdb, file.c_str());
        return;
    }

    m_File2Module[configFile].erase(iter);
}

void ConfigerManager::saveConfigure()
{
    for(auto& mm : m_File2Module)
    {
        for(auto& cb : mm.second)
        {
            cb.first->save(cb.second, mm.first);
        }
    }

    for(auto& sf : m_Str2File)
    {
        sf.second->save();
    }
}

void ConfigerManager::reloadConfigure()
{
    for(auto& n : m_Str2File)
    {
        reloadConfigure(n.first);
    }
}

void ConfigerManager::reloadConfigure(std::string name)
{
    if(m_Str2File.find(name) == m_Str2File.end())
    {
        std::stringstream ss;
        ss << "no configure file called " << name;
        throw std::logic_error(ss.str());
    }

    m_Str2File[name]->reload();

    if(m_File2Module.find(m_Str2File[name]) == m_File2Module.end())
    {
        return;
    }

    for(auto& m : m_File2Module[m_Str2File[name]])
    {
        m.first->reload(m.second, m_Str2File[name]);
    }

    m_Str2File[name]->save();
}

NAMESPACE_AFL_BASE_END
