#include "../include/Utility.h"

void trim(std::string& str)
{
    while (!str.empty() && std::isspace(str[0]))
    {
        str.erase(0, 1);
    }
    while (!str.empty() &&std::isspace(str[str.length()-1]))
    {
        str.erase(str.length()-1, 1);
    }
}


std::unique_ptr<Lock> Lock::m_instance;
std::once_flag Lock::m_onceFlag;
 
Lock& Lock::GetInstance()
{
    std::call_once(m_onceFlag,
        [] {
            m_instance.reset(new Lock);
    });
    return *m_instance.get();
}