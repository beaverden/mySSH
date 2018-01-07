#include "../include/Evaluate.h"
#include "../include/Parser.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#include <memory>

int main(int argc, char* argv)
{
    if (argc < 2)
    {
        printf("Usage: ./shell command");
    }

    bool newMessage = true;
    std::shared_ptr<ExecutionContext> ctx = std::make_shared<ExecutinContext>();
    try
    {
        std::string comm = argv[1];
        Evaluate(comm)
    }

    while (true)
    {
        if (ctx->shouldTerminate)
        {
            std::string bye = "[Shell] Bye.";
            write(ctx->sv_prog, bye.c_str(), bye.length());
            return;
        }

        if (newMessage)
        {
            ctx->sslMutex.lock();
            write(ctx->sv_prog, "denis@ubuntu$ ", strlen("denis@ubuntu$ "));
            ctx->sslMutex.unlock();
            newMessage = false;
        }

        int has_read;
        int result = 0;
        ioctl(ctx->sv_prog, FIONREAD, &has_read);
        if (has_read > 0)
        {
            try
            {
                char* buff = new char[has_read + 1]();
                if (buff == nullptr) throw std::bad_alloc();
                result = read(ctx->sv_prog, buff, has_read);
                std::string comm = buff;
                delete buff;
                Evaluate(comm, ctx);
                newMessage = true;                
            }
            catch (ParserException& ex)
            {
                send_error(ctx, ex.what());
                newMessage = true;  
                continue;
            }
            catch (VerificationException& ex)
            {
                send_error(ctx, ex.what()); 
                newMessage = true;  
                continue;              
            }
            catch (EvaluationException& ex)
            {
                send_error(ctx, ex.what());
                newMessage = true; 
                continue;
            }
            catch (ExitException& ex)
            {
                ctx->shouldTerminate = true;
                continue;
            }
            catch (std::exception& ex)
            {
                send_error(ctx, ex.what());
                newMessage = true; 
                continue;
            }
            catch (...)
            {
                send_error(ctx, "[Shell] Unknown exception");
                continue;
            }
        }
    }              
}