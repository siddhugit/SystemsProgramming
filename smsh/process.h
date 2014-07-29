/*
*   Declaration of public interface of process module
*/

//process mode
enum PROCESSMODE {INTERACTIVE,SCRIPT};

int processInput(FILE*);
void setMode(enum PROCESSMODE);
const char* getPrompt();
