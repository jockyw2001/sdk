#ifndef MDB_INTEGRATE
#define MDB_INTEGRATE
#include "Mdb_Base.h"
#include "Live555RTSPServer.hh"


class Mdb_Integrate : public Mdb_Base
{
    public:
        Mdb_Integrate();
        virtual ~Mdb_Integrate();
        virtual void ShowWelcome(std::string &strOut);
    private:
        void VpeVifInit(std::vector<std::string> &inStrings, std::string &strOut);
        void VpeVifDeinit(std::vector<std::string> &inStrings, std::string &strOut);
        void VpeWriteFile(std::vector<std::string> &inStrings, std::string &strOut);
        void VpeCreatePort(std::vector<std::string> &inStrings, std::string &strOut);
        void VpeDestroyPort(std::vector<std::string> &inStrings, std::string &strOut);

        void VencInit(std::vector<std::string> &inStrings, std::string &strOut);
        void VencDeinit(std::vector<std::string> &inStrings, std::string &strOut);
        void VencWriteFile(std::vector<std::string> &inStrings, std::string &strOut);
        void VencInjectFrame(std::vector<std::string> &inStrings, std::string &strOut);

        void RtspServerStart(std::vector<std::string> &inStrings, std::string &strOut);
        void RtspServerStop(std::vector<std::string> &inStrings, std::string &strOut);

        static void* OpenStream(char const* szStreamName, void* arg);
        static int VideoReadStream(void* handle, unsigned char* ucpBuf, int BufLen, struct timeval *p_Timestamp, void* arg);
        static int CloseStream(void* handle, void* arg);

        static std::map<std::string, Live555RTSPServer*> mapRTSPServer;
};
#endif
