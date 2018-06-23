#ifndef MDB_BASE_H
#define MDB_BASE_H
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <map>

#include <iostream>
#include <sstream>

typedef enum
{
    MDB_MODULE_RGN = 1,
    MDB_MODULE_INTEGRATE = 2,
    MDB_MODULE_MAX
}MDB_MODULE;

#define MDB_MODULE_RGN_NAME "region"
#define MDB_MODULE_INTEGRATE_NAME "integrate"


#ifndef ASSERT
#define ASSERT(_x_)                                                                         \
    do  {                                                                                   \
        if ( ! ( _x_ ) )                                                                    \
        {                                                                                   \
            printf("ASSERT FAIL: %s %s %d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__);     \
            abort();                                                                        \
        }                                                                                   \
    } while (0)
#endif
#ifndef PTH_RET_CHK
#define PTH_RET_CHK(_pf_) \
    ({ \
        int r = _pf_; \
        if ((r != 0) && (r != ETIMEDOUT)) \
            printf("[PTHREAD] %s: %d: %s: %s\n", __FILE__, __LINE__, #_pf_, strerror(r)); \
        r; \
    })
#endif

typedef enum
{
    PRINT_COLOR_NORMAL = 0,
    PRINT_COLOR_BLACK = 30,
    PRINT_COLOR_RED ,
    PRINT_COLOR_GREEN,
    PRINT_COLOR_YELLOW,
    PRINT_COLOR_BLUE,
    PRINT_COLOR_FUNCHSIN,
    PRINT_COLOR_CYAN,
    PRINT_COLOR_WHITE
}PRINT_COLOR;
typedef enum
{
    PRINT_MODE_NORMAL = 0,
    PRINT_MODE_HIGHTLIGHT = 1,
    PRINT_MODE_UNDERLINE = 4,
    PRINT_MODE_FLICK = 5,
    PRINT_MODE_INVERT = 7,
}PRINT_MODE;

#define PREPARE_MODULE(module_enum, module_name) do{ \
    std::stringstream sstr; \
    sstr << '[' << module_enum << "] " << module_name << std::endl; \
    Print(strOut, sstr.str(), PRINT_COLOR_YELLOW, PRINT_MODE_HIGHTLIGHT);    \
}while (0);
#define PREPARE_COMMAND(cmdStr, fpFunction, max_para)    do{ \
    mapMdbBaseCmd[cmdStr].fpFunc = (SUBCMD_FUNC)(fpFunction);    \
    mapMdbBaseCmd[cmdStr].maxPara = max_para;    \
}while (0);
#define START_PROCESS   do{    \
    pthread_create(&tid, NULL, MdbProcess, this); \
}while(0);
#define STOP_PROCESS    do{    \
    bExit = 1;  \
    pthread_join(tid, NULL); \
}while(0);

#define INSTALL_PROCESS(processName, fpFunction, pAttr)    do{ \
    pthread_mutex_lock(&mutexProcess);  \
    mapCmdProcess[processName].fpFunc = (SUBPROCESS_FUNC)fpFunction;    \
    mapCmdProcess[processName].pArg = pAttr;    \
    pthread_mutex_unlock(&mutexProcess);  \
}while (0);

#define UNINSTALL_PROCESS(processName)    do{ \
    std::map<std::string, MDB_Process_t>::iterator iter;    \
    pthread_mutex_lock(&mutexProcess);  \
    iter = mapCmdProcess.find(processName);     \
    if (iter != mapCmdProcess.end()) \
    {   \
        mapCmdProcess.erase(iter);    \
    }   \
    pthread_mutex_unlock(&mutexProcess);  \
}while (0);

#define MDB_EXPECT(testcaseName, testcaseStr, testcaseExp) do {    \
        std::string tmpStr;    \
        char number[20];    \
        tmpStr += testcaseName;    \
        unsigned int intCurrentTime = GetTime();    \
        if (testcaseExp)    \
        {   \
            tmpStr += " : Test ok ";    \
            sprintf(number, "T: %d\n", GetTimeDiff(intCurrentTime));    \
            tmpStr += number;   \
            Print(testcaseStr, tmpStr, PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT);    \
        }   \
        else    \
        {   \
            tmpStr += " : Test fail!\n";    \
            Print(testcaseStr, tmpStr, PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT);    \
        }   \
    }while (0);
#define MDB_EXPECT_ERRCASE(testcaseName, testcaseStr, testcaseExp, errorcase) do {    \
        std::string tmpStr;    \
        char number[20];    \
        tmpStr += testcaseName;    \
        unsigned int intCurrentTime = GetTime();    \
        if (testcaseExp)    \
        {   \
            tmpStr += " : Test ok ";    \
            sprintf(number, "T: %d\n", GetTimeDiff(intCurrentTime));    \
            tmpStr += number;   \
            Print(testcaseStr, tmpStr, PRINT_COLOR_GREEN, PRINT_MODE_HIGHTLIGHT);    \
        }   \
        else    \
        {   \
            tmpStr += " : Test fail!\n";    \
            Print(testcaseStr, tmpStr, PRINT_COLOR_RED, PRINT_MODE_HIGHTLIGHT);    \
            {errorcase;};   \
        }   \
    }while (0);

#define MDB_EXPECT_OK(testcaseName, testcaseStr, ret, expval) MDB_EXPECT(testcaseName, testcaseStr, (ret == expval))
#define MDB_EXPECT_FAIL(testcaseName, testcaseStr, ret, expfailval) MDB_EXPECT(testcaseName, testcaseStr, (ret != expfailval))
#define MDB_EXPECT_OK_ERRCASE(testcaseName, testcaseStr, ret, expval, errcase) MDB_EXPECT_ERRCASE(testcaseName, testcaseStr, (ret == expval), errcase)
#define MDB_EXPECT_FAIL_ERRCASE(testcaseName, testcaseStr, ret, expfailval, errcase) MDB_EXPECT_ERRCASE(testcaseName, testcaseStr, (ret != expfailval), errcase)

class Mdb_Base
{


    public:
        Mdb_Base();
        virtual ~Mdb_Base();
        static Mdb_Base *GetInstance()
        {
            return pInstance;
        }
        static void Create(MDB_MODULE enModule);
        static void PrepareModule(std::string &strOut);
        static unsigned int Atoi(std::string &strOut);
        static void Print(std::string &strOut, std::string strContainer, PRINT_COLOR enColor = PRINT_COLOR_NORMAL, PRINT_MODE enMode = PRINT_MODE_NORMAL);
        void SetPara(std::string &strCmd, std::vector<std::string> &strInStrings);
        void GetName(std::string &strOut); //Get module name
        void DumpCmd(std::string &strOut);

        typedef void (Mdb_Base::*SUBCMD_FUNC)(std::vector<std::string> &, std::string &);
        typedef void (Mdb_Base::*SUBPROCESS_FUNC)(void *);

        typedef struct MDB_Cmd_s
        {
            SUBCMD_FUNC fpFunc;
            unsigned char maxPara;
        }MDB_Cmd_t;


        typedef struct MDB_Process_s
        {
            SUBPROCESS_FUNC fpFunc;
            void * pArg;
        }MDB_Process_t;


        void DoCmd(std::string &strOut)
        {
            std::map<std::string, MDB_Cmd_t>::iterator it;
            it = mapMdbBaseCmd.find(strMdbBaseSubCmd);
            if (it != mapMdbBaseCmd.end())
            {
                if (it->second.maxPara != strMdbBaseInStrings.size())
                {
                    Print(strOut, "Cmd Para error!\n", PRINT_COLOR_YELLOW, PRINT_MODE_HIGHTLIGHT);
                    return;
                }
                (this->*(it->second.fpFunc))(strMdbBaseInStrings, strOut);
            }
        }
        virtual void ShowWelcome(std::string &strOut)
        {
            printf("%s\n", __FUNCTION__);
        }
        void Destroy(void)
        {
            printf("%s\n", __FUNCTION__);
            delete this;
        }
        template<class MDBCHILD>
        class Mdb_Setup
        {
            public:
                explicit Mdb_Setup(const char *pModuleName);
                ~Mdb_Setup()
                {
                    ;//printf("%s\n", __FUNCTION__);
                }
        };
    protected:
        static void *MdbProcess(void *arg)
        {
            Mdb_Base *pIns = (Mdb_Base *)arg;
            std::map<std::string, MDB_Process_t>::iterator iter;
            if (pIns)
            {
                while (pIns->bExit == 0)
                {
                    pthread_mutex_lock(&pIns->mutexProcess);
                    for (iter = pIns->mapCmdProcess.begin(); iter != pIns->mapCmdProcess.end(); iter++)
                    {
                        (pIns->*(iter->second.fpFunc))(iter->second.pArg);
                    }
                    pthread_mutex_unlock(&pIns->mutexProcess);
                    usleep(40);
                }
            }
            return NULL;
        }
        std::map<std::string, MDB_Cmd_t> mapMdbBaseCmd;
        std::map<std::string, MDB_Process_t> mapCmdProcess;
        pthread_mutex_t mutexProcess;
        unsigned int GetTime(void);
        unsigned int GetTimeDiff(unsigned int intOldTime);

    private:
        std::vector<std::string> strMdbBaseInStrings;
        std::string strMdbBaseSubCmd;
        static Mdb_Base* pInstance;
        std::string strMdbBaseSubName;
        pthread_t tid;
        int bExit;
};

#endif //MDB_BASE_H
