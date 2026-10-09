/*
 * @Author: cs
 * @Date: 2024-02-01 11:16:47
 * @LastEditors: zhangenwei
 * @LastEditTime: 2024-02-01 14:16:47
 * @Description:
 */
#ifndef COMM_CONFIGURE_CONFIGERDATA_H_
#define COMM_CONFIGURE_CONFIGERDATA_H_

#include "configer_file.h"
#include "base/common/network/serializable_data.h"

NAMESPACE_AFL_BASE_START

class ConfigerDataBase : public afl::base::SerializableData
{
public:
    virtual void reload(std::string name, std::shared_ptr<ConfigerFile> cf) = 0;
    virtual void save(std::string name, std::shared_ptr<ConfigerFile> cf) = 0;

    virtual const std::string& getFileName() const
    {
        return m_FileName;
    }

    virtual void setFileName(const std::string& fileName)
    {
        m_FileName = fileName;
    }

private:
    std::string m_FileName;
};

template<typename T>
class ConfigerData : public ConfigerDataBase
{
    using UpdateCallBack = std::function<void(T& data)>;

public:
    explicit ConfigerData(std::string comment = "", UpdateCallBack cb = nullptr)
            : m_Comment(comment), f_CallBack(cb), m_NeedPatchConfig(false)
    {}

    virtual void save(std::string name, std::shared_ptr<ConfigerFile> cf)
    {
        cf->configBlock(name) = *(static_cast<T*>(this));
        cf->save();
    }

    virtual void reload(std::string name, std::shared_ptr<ConfigerFile> cf)
    {
        try
        {
            auto& cb = cf->configBlock(name);

            T newData = cb;
            newData.setUpdateCallBack(this->f_CallBack);
            newData.setFileName(this->getFileName());

            *(static_cast<T*>(this)) = newData;

            if(f_CallBack)
                f_CallBack(*(static_cast<T*>(this)));

            cb = *(static_cast<T*>(this));
        }
        catch(ConfigBlock::exception& e)
        {
            cf->configBlock(name) = *(static_cast<T*>(this));
            std::cout << "update configure block to data error!" << e.what() << std::endl;
        }
    }

private:
    virtual void serialize(nlohmann::json& j)
    {
        writeToFile((ConfigBlock&)j);

        if(!m_Comment.empty())
            j["_comment"] = m_Comment;
    }

    virtual void deserialize(const nlohmann::json& j)
    {
        readFromFile((ConfigBlock&)j);
    }

    virtual void writeToFile(ConfigBlock& cb) = 0;
    virtual void readFromFile(const ConfigBlock& cb) = 0;

public:
    void setUpdateCallBack(UpdateCallBack cb)
    {
        f_CallBack = cb;
    }

    UpdateCallBack& getUpdateCallBack()
    {
        return f_CallBack;
    }

    bool needPatchConfig()
    {
        return m_NeedPatchConfig;
    }

private:
    std::string m_Comment;
    UpdateCallBack f_CallBack;
protected:
    bool m_NeedPatchConfig;
};

NAMESPACE_AFL_BASE_END
#endif /* COMM_CONFIGURE_CONFIGERDATA_H_ */
