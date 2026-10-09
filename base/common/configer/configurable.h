/*********************************************************************************
 * @file		configurable.h
 * @brief		configurable belongs to CICTCI
 * @details
 * @author		cs
 * @date		2024-02-01
 * @copyright	Copyright (c) 2024 CICTCI-Airos Division.
 * @verbatim
 *
 *  Change History:
 *  Date      Author    Version  ChangeId           Description
 *  ------------------------------------------------------------------------------
 *  2014-02-01 cs       1.0       ————             Create this file
 *  2024-02-12 zhangenwei  modify
 * @endverbatim
 ********************************************************************************/
#ifndef BASE_CONFIGER_CONFIGURABLE_H_
#define BASE_CONFIGER_CONFIGURABLE_H_

#include "configer_manager.h"

NAMESPACE_AFL_BASE_START

template<typename T>
class Configurable
{
public:
    using ConfigerType = T;

public:
    Configurable(std::string name, std::string dir = std::string(), std::string fileName = std::string())
//            : m_key(name), m_ConfigerManager(new ConfigerManager(dir))
    {
        static_assert( std::is_base_of<ConfigerData<T>, T>::value, "Configure Type (T) Must be Derived from ConfigerData<T>!");
        if(!name.empty())
        {
            m_key  = name;
        }
        if(!dir.empty())
        {
            std::string rootDir = dir;
            m_ConfigerManager = std::unique_ptr<ConfigerManager>(new ConfigerManager(rootDir));
        }

        m_IndexKey = m_ConfigerManager->registerConfiger(m_key, m_Configer, fileName);

        if(m_Configer.needPatchConfig())
        {
            saveConfiger();
        }
    }

    virtual ~Configurable()
    {
        m_ConfigerManager->unregisterConfiger(m_IndexKey);
    }

    ConfigerType& getConfiger()
    {
        return m_Configer;
    }

    void setConfiger(ConfigerType& configer)
    {
        configer.setFileName(m_Configer.getFileName());
        configer.setUpdateCallBack(m_Configer.getUpdateCallBack());
        m_Configer = configer;
    }

    void saveConfiger()
    {
        m_Configer.save(m_key,  m_ConfigerManager->getConfigerFile(m_IndexKey));
    }

protected:
    ConfigerType m_Configer;

private:
    std::unique_ptr<ConfigerManager> m_ConfigerManager;
    std::string m_key;
    std::uint64_t m_IndexKey;
};

NAMESPACE_AFL_BASE_END

#endif /* BASE_CONFIGER_CONFIGURABLE_H_ */
