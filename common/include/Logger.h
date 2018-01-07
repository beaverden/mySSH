/** @file */

#ifndef LOGGER_H
#define LOGGER_H

/** @brief Log basic data about packets, but not the content */
#define LOG_PACKETS         (1 << 0)
/** @brief Log packet content */
#define LOG_PACKET_DATA     (1 << 1)
/** @brief Log when a client has connected */
#define LOG_CONNECTIONS     (1 << 2)
/** @brief Log exceptions and errors */
#define LOG_ERRORS          (1 << 3)
/** @brief Don't log */
#define LOG_NONE            (1 << 4)
/** @brief Log important events during the protocol */
#define LOG_EVENTS          (1 << 5)
/** @brief Log evaluation events */
#define LOG_EVAL            (1 << 6)
/** @brief Log detailed evaluation */
#define LOG_EVAL_VERBOSE    (1 << 7)

#define CHECK(opts, flg)    (((opts) & (flg)) != 0)

#include <string>
#include <string.h>
#include <memory>
#include <fstream>
#include <ctime>
#include <mutex>

/**
 * \brief Class to log formatted information into a log file
 * 
 * Every time Log will be called, log_mutex will be placed on
 * the file and information will be logged along with the current time
 */
class Logger
{
public:
    /**
     * \brief Log formatted data with variable number of arguments (simmilar to printf)
     *
     * \param[in] fmt printf syntax format string
     * \param[in] args Format arguments
     */
    template<class ... Args>
    static void log(const char* fmt, Args ... args)
    {
        char finalArr[1000] = {0};
        snprintf(finalArr, 999, fmt, args...);
        time_t t = time(0);   // get time now
        struct tm * now = localtime( & t );
        char date[1000] = {0};
        snprintf(date, 999, "[%d-%d-%d %d:%d:%d] ",
            now->tm_mday, now->tm_mon + 1, now->tm_year + 1900,
            now->tm_hour, now->tm_min, now->tm_sec
        );
        try
        {
            logMutex.lock();
            std::ofstream file(logPath, std::ios_base::app);
            file.write(date, strlen(date));
            file.write(finalArr, strlen(finalArr));   
            file.write("\n", 1);
            file.close();
            logMutex.unlock();
        }
        catch (...) {}
    }

    /**
     * \brief Log only if the specific flag with prefix LOG is set
     * 
     * \param[in] flgs Flag to be checked
     * \param[in] fmt printf syntax format string
     * \param[in] args Format arguments
     */
    template<class ... Args>
    static void log(unsigned int flg, const char* fmt, Args ... args)
    {
        if (CHECK(logOptions, flg))
        {
            log(fmt, args...);
        }
    }
    
    /**
     * \brief Truncates the log file located at [log_path]
     */
    static void resetLog()
    {
        try
        {
            std::ofstream file(logPath, std::ios_base::trunc);
            file.close();
        }
        catch (...) {}
    }

    static unsigned int     logOptions;    ///< Mask of flags starting with LOG_
    static std::string      logPath;       ///< Path to log file
    static std::mutex       logMutex;
};


#endif //LOGGER_H