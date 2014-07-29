/*
*   Declaration of public interface of splitline module
*/

char * next_cmd(const char *prompt, FILE *fp);
char* varSubstitute(const char* cmdLine);
char ** splitline(char *line);
char *newstr(char *s, int l);
void freelist(char **list);
void * emalloc(size_t n);
void * erealloc(void *p, size_t n);

