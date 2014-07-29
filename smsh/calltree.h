/*
* Declares public interface of calltree.c
*/

int buildCallTree(char** arglist, FILE* fp);
int processCallTree();
int is_control_command(const char*);
