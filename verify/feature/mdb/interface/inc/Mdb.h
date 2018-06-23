#ifndef MDB_H//MDB_H
#define MDB_H
#include <pthread.h>

#include <vector>
#include <string>

class Mdb
{
   public:
    explicit Mdb()
    {
        bExitTransThread = 0;
        return;
    }
    virtual ~Mdb()
    {
        return;
    }
    static void Run(void);
    static void Wait(void);
   private:
    void ProcessStrings(char *pInStr, char *pOutStr);
    void ParseStrings(char *pStr);
    void ShowWelcome(void);
    void TransGetData(void);
    std::vector<std::string> strInStrings;
    std::string strSubCmd;
    std::string strOutString;
    unsigned char bExitTransThread;
    pthread_attr_t tAttr;
    pthread_t tTid;

    static void *TransThread(void *pPara);
    static Mdb *pInstance;
};
#endif //MDB_H
