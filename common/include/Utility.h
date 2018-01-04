

#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <memory>
#include <mutex>
/*
* Trims a string of spaces and tabs
*/
void trim(std::string& str);

class Lock
{
public:
    virtual ~Lock() = default;
    static Lock& GetInstance();
    void Set() { while(flag) {}; flag = true; }
    void Reset() { flag = false; }
private:
    bool flag;
    static std::unique_ptr<Lock> m_instance;
    static std::once_flag m_onceFlag;
    Lock() = default;
    Lock(const Lock& src) = delete;
    Lock& operator=(const Lock& rhs) = delete;
};

#endif //UTILITY_H