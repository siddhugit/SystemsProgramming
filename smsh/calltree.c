/*
*   Creates, processes and destroys the call tree.
*   Contains data structure defintion for the call tree
*   and implements algorithms to create, process and
*   destroy the call tree. The call tree ia used to 
*   process control structure ( if else ) in the program.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "splitline.h"

//type of command sequence element
enum Commandtype { COMMAND,NESTED_IF };
//states of program execution in if block 
enum ControlState { NEUTRAL,WANT_THEN,WANT_ELSE_FI,WANT_FI};
//type of control commands
enum ControlType { T_NA,T_IF,T_THEN,T_ELSE,T_FI};

//control command text and type map
struct ControlData
{
    char* ctrlText;
    enum ControlType ctrlType;
};

//data structure of control command map
static struct ControlData cntrlwords[] = 
                    { {"if", T_IF}, {"then", T_THEN}, {"fi", T_FI},
			        {"else",T_ELSE},{NULL,0} };


//Class for command sequece element
struct CommandSequence
{
    void* cmd;//command data
    enum Commandtype type;//command type ( command or another NESTED_IF block)
    struct CommandSequence* next;//next element
};

typedef struct CommandSequence COMMSEQ;

//class for if block
struct IfBlock
{
    char** commandargs;//if command
    COMMSEQ* thenComSeq;//command sequence for then block
    COMMSEQ* elseComSeq;//command sequence for else block
};

typedef struct IfBlock IFBLOCK;

//The root of call tree
static IFBLOCK* the_if_block = NULL;

/*
 * purpose: boolean to report if the command is a shell control command
 * returns: 1 - if shell control command
 *          0 - otherwise 
 */
int is_control_command(const char *s)
{
	int	i;

	for(i=0; cntrlwords[i].ctrlText != NULL ; i++ )
	{
		if ( strcmp(s, cntrlwords[i].ctrlText) == 0 )
			return 1;//if found in control command map
	}
	return 0;//not found
}

/*
 * parameters: char* str
 * returns: type of control command str is
 *          T_IF,T_ELSE,T_THEN,T_THEN,T_NA 
 */
static enum ControlType getCtrlType(char* str)
{
    int	i;

	for(i=0; cntrlwords[i].ctrlText != NULL ; i++ )
	{
		if ( strcmp(str, cntrlwords[i].ctrlText) == 0 )
			return cntrlwords[i].ctrlType;//returns found tyoe
	}
	return T_NA;//not found
}

/*
 * Executes command while processing call tree   
 * parameters: char** args - command arguments
 * returns: exit status of command executed
 */
static int execCommand(char** args)
{
    int rv = 0;//return exit status
    int execute(char**);//execute() function declaration
    int builtin_command(char**,int*);//built in handler function declaration 
    
    if ( !builtin_command(args,&rv) )//handles built in command
        rv = execute(args);//regular command
    return rv;//returns exit status of command executed
}

/*
*   Prints error message and return 1 indicating failure
*/
static int errorMsg(const char* msg)
{
    fprintf(stderr,"%s\n",msg);
    return 1;
}

/*
*   Deallocates tree memory.
*   parameters: IFBLOCK* ifBlock - destroys tree rooted at ifBlock 
*/
static void clearTree(IFBLOCK* ifBlock)
{
    COMMSEQ *node,*tempNode;//for linked list iteration
    COMMSEQ* nodes[2];//then and else nodes
    int i;//loop counter
    IFBLOCK* currIfBlock;//nested if block
    char* command;//sequence command
    
    nodes[0] = ifBlock->thenComSeq;//then command sequesnce
    nodes[1] = ifBlock->elseComSeq;//else command sequence
    for(i = 0; i <2; ++i){
        node = nodes[i];
        while(node != NULL){
            if(node->type == COMMAND){
                tempNode = node->next;
                command = (char*)node->cmd;
                free(command);
                free(node);//free command sequence node
                node = tempNode;
            }
            else{//if nested_if
                currIfBlock = (IFBLOCK*)node->cmd;//get if block
                clearTree(currIfBlock);//free nested if block recursively
                tempNode = node->next;
                free(command);
                free(node);//then free command sequence node
                node = tempNode;
            }
        }
    }
    free(ifBlock);//free if block
}

/*
*   processes call tree.
*   runs if command, if successful run commands in then block otherwise 
*   runs commands in else block. 
*   calls itself if command is nested_if block
*   parameters: IFBLOCK* ifBlock - processes tree rooted at ifBlock 
*   returns : 0 if success otherwise 1
*/
static int processTree(IFBLOCK* ifBlock)
{
    int result;//result of processing
    COMMSEQ* node;//for linked list iteration
    IFBLOCK* currIfBlock;//poiner to nested if block
    char* command,**arglist;//for convenience
    
    result = execCommand(ifBlock->commandargs + 1);//result of if commad
    node = (result == 0) ? ifBlock->thenComSeq 
                            : ifBlock->elseComSeq;//which block to execute
    while(node != NULL)
    {
        if(node->type == COMMAND){//executable command
            command = (char*)node->cmd;
            arglist = splitline(command);//get command args
            result = execCommand(arglist);//execute command
        }
        else{//nested_if block
            currIfBlock = (IFBLOCK*)node->cmd;
            result = processTree(currIfBlock);//recursively process nested if
        }
        node = node->next;//go to next node
    }
    return result;
}

/*
*   Utility function to create and initialize new if block.
*   its a helper function for buildtree().
*   parameters: char** arglist - if command
*   returns : pointer to newly created if block  
*/
static IFBLOCK* getNewIfBlock(char** arglist)
{
    IFBLOCK* newIfBlock = malloc(sizeof(IFBLOCK));
    newIfBlock->commandargs = arglist;//set command args
    newIfBlock->thenComSeq = NULL;//initialize command sequence to NULL
    newIfBlock->elseComSeq = NULL;//initialize command sequence to NULL
    return newIfBlock;
}

/*
*   Utility function to create and initialize new command sequence element
*   sequence element. its a helper function for buildtree().
*   parameters: enum Commandtype commtype - type of control command
*   returns : pointer to newly created command sequence element 
*/
static COMMSEQ* getNewComSeq(enum Commandtype commtype)
{
    COMMSEQ* comSeq = malloc(sizeof(COMMSEQ));
    comSeq->type = commtype;//set control commad type
    comSeq->next = NULL;//initialize next element to NULL 
    return comSeq;
}

/*
*   Creates call tree. 
*   Reads line by line from input, checks the type of line
*   A nested Ifblock has parent COMMSEQ. sets the COMMSEQ
*   as parent of if block. the_if_block points to top level
*   if block.
*   each if block has sequence of then and else commands.
*   the sequence is represented as linked list. The elements
*   of sequence can be a regular command or nested_if.   
*   creates a command sequence node when a then/else command
*   is encountered. if command is nested if block calls itself
*   to build tree for nested if block and passes the command sequence
*   node as parent. keeps poiter to current node in order to set
*   its next element.
*   parameters: FILE* fp - input file
*               IFBLOCK* ifBlock - current if block
*               COMMSEQ* parent - parent of ifblock
*   returns : 0 if success otherwise 1
*/
static int buildTree(FILE* fp,IFBLOCK* ifBlock,COMMSEQ* parent)
{
    enum ControlType type;//control type
    enum ControlState state = WANT_THEN;//control staes
    COMMSEQ **currSeq,*tempSeq;//current node and a temp node
    IFBLOCK* newIfBlock;//for programming convenience
    char **arglist,*cmdline;
    int ret = 0;
    const char* getPrompt();
    
    if( parent != NULL){
        parent->cmd = (void*)ifBlock;
    }
    while((cmdline = next_cmd(getPrompt(), fp)) != NULL){//get next line
        cmdline = varSubstitute(cmdline); //variable substituion
        arglist = splitline(cmdline);
        if(arglist[0] != NULL){
            type = getCtrlType(arglist[0]);
            if(type == T_IF){//nested if
                tempSeq = getNewComSeq(NESTED_IF);//create new comm seq node
                newIfBlock = getNewIfBlock(arglist);//create new if block
                *currSeq = tempSeq;//update currseq
                currSeq = &tempSeq->next;
                ret = buildTree(fp,newIfBlock,tempSeq);//build tree recursively
            }
            else if(type == T_THEN){//Then block
                if(state != WANT_THEN)
                    return errorMsg("unexpected then");//something wrong
                state = WANT_ELSE_FI;
                currSeq = &(ifBlock->thenComSeq);//update currseq to then seq
            } 
            else if(type == T_ELSE){//else
                if(state != WANT_ELSE_FI)
                    return errorMsg("unexpected else");//something wrong
                state = WANT_FI;
                currSeq = &(ifBlock->elseComSeq);//update currseq to then seq
            } 
            else if(type == T_FI){//fi
                if(state != WANT_FI && state != WANT_ELSE_FI)
                    return errorMsg("unexpected fi");//something wrong
                return ret;//return when fi is encountered
            }
            else{//then/else regular commands
                tempSeq = getNewComSeq(COMMAND);//new comm seq node
                tempSeq->cmd = (void*)cmdline;
                *currSeq = tempSeq;
                currSeq = &tempSeq->next;//update currseq
            }
        }
    }//something wrong EOF is encountered, expected:fi
    return ( (cmdline == NULL) ? errorMsg("unexpected") : ret );    
}

/*
*   Destroys tree and sets the_if_block to NULL
*/
static void clearCallTree()
{
    clearTree(the_if_block);//deallocate tree
    the_if_block = NULL;//set it back to NULL
}

/*
*   public function:Initializes the_if_block
*   calls private function buildTree() to build call tree
*   if buildtree() fails clearCallTree() is called to destroy tree
*   parameters: FILE* fp - input file
*               char** arglist - top level if block command arg
*   returns : 0 if success otherwise 1
*/
int buildCallTree(char** arglist, FILE* fp)
{
    int ret;
    
    if(the_if_block == NULL){
        the_if_block = malloc(sizeof(IFBLOCK));//initialize if block
    }
    the_if_block->commandargs = arglist;
    the_if_block->thenComSeq = NULL;
    the_if_block->elseComSeq = NULL;
    ret = buildTree(fp,the_if_block,NULL);//build tree
    if(ret == 1)
        clearCallTree();//destroy tree if build tree fails
    return ret;
}

/*
*   public function:
*   calls private function processTree() to process call tree
*   calls clearCallTree()  to destroy the tree
*   returns : 0 if success otherwise 1
*/
int processCallTree()
{
    int ret;
    
    ret = processTree(the_if_block);//process tree
    clearCallTree();//destroy tree
    return ret;
}


