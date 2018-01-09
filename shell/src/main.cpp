#include "../include/Evaluate.h"
#include "../include/Parser.h"
#include "../include/Exception.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <memory>

int main(int argc, char* argv[])
{
    // TODO fix shell grep "main" error
    if (argc != 1)
    {
        printf("Usage: ./shell");
        return 0;
    }

    std::shared_ptr<ExecutionContext> ctx = std::make_shared<ExecutionContext>();
    ctx->inputRedir.push(STDIN_FILENO);
    ctx->outputRedir.push(STDOUT_FILENO);
    ctx->errorRedir.push(STDERR_FILENO);
    ctx->currentDir = getCwd();
    int hasRead = 0;
    int readResult = 0;
    bool newMessage = true;
    while (true)
    {  
        try
        {
            if (newMessage)
            {
                std::string pathStr = getUsername() + ":" + getCwd() + "$ ";
                write(STDOUT_FILENO, pathStr.c_str(), pathStr.length());
                newMessage = false;
            }

            ioctl(STDIN_FILENO, FIONREAD, &hasRead);
            if (hasRead > 0)
            {
                char* buff = new char[hasRead + 1]();
                if (buff == nullptr) throw std::bad_alloc();
                readResult = read(STDIN_FILENO, buff, hasRead);
                std::string comm = buff;
                delete buff;
                Evaluate(comm, ctx);
                newMessage = true;                
            }
        }
        catch (ExitException& ex)
        {
            ctx->shouldTerminate = true;
            printf("[Shell] Bye.\n");
            fflush(stdout);
            break;
        }
        catch (AuthException& ex)
        {

            printf("[Shell] %s\n", ex.what());
            fflush(stdout);
            newMessage = true;
            continue;
        }
        catch (ParserException& ex)
        {
            printf("[Shell] %s\n", ex.what());
            fflush(stdout);
            newMessage = true;
            continue;
        }
        catch (VerificationException& ex)
        {
            printf("[Shell] %s\n", ex.what()); 
            fflush(stdout); 
            newMessage = true;
            continue;         
        }
        catch (EvaluationException& ex)
        {
            printf("[Shell] %s\n", ex.what());
            fflush(stdout);
            newMessage = true;
            continue; 
        }
        catch (std::exception& ex)
        {
            printf("[Shell] %s\n", ex.what());
            fflush(stdout);
            newMessage = true;
            continue;
        }
        catch (...)
        {
            printf("[Shell] Unknown exception\n");
            fflush(stdout);
            newMessage = true;
            continue;
        }
    }
    return 0;        
}