#include <limits.h>

class Script
{
private:
    char dirNameIn[PATH_MAX], dirNameOut[PATH_MAX];
    char configPath[PATH_MAX];
    int inoInit, watchDesc;

public:
    Script(char* configName);
    ~Script();
    bool readConfig();
    bool reReadConfig();
    bool perform();
    bool haveChanges();
};
