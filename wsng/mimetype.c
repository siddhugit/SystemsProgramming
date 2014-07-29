/*
* implements table driven data structure for mime types.
* Stores mime name and type in a dynamic array and
* exposes add and search interface for adding and finding
* mime types in the array
*/

#include <stdlib.h>
#include <string.h>
#include "mimetype.h"

//name and type of mimetypes
struct fileType
{
    const char* type;
    const char* name;
};

//dynamic array of mimetypes
struct typeVector
{
    struct fileType* files;//array of name/type pair
    int size;//number of types array holding
    int capacity;//number of types array can hold 
};


static struct typeVector typeV = {NULL,0,0};//initialize array

/*
*   called  when array does not hold anything
*   initializes array such that it can hold
*   n elements
*/
static void initV(int n)
{
    int sz;
    
    typeV.capacity = n;
    typeV.size = 0;
    sz = typeV.capacity * sizeof(struct fileType);//allocate memory
    typeV.files = malloc(sz);
    memset(typeV.files,0,sz);
}

/*
* clears memory when array needs expansion
*/
static void clearfileTypeArray()
{
    int i = 0;
    
    for(; i < typeV.size; ++i){
        free((char*)typeV.files[i].type);
        free((char*)typeV.files[i].name);
    }
    free(typeV.files);
}

/*
*   double the capacity of dynamic array
*   and deallocates old storage
*/
static void expand()
{
    int sz = 2*typeV.capacity;
    //allocate new memory of double the previous capacity
    struct fileType* temp = malloc(sz * sizeof(struct fileType));
    memset(temp,0,sz * sizeof(struct fileType));
    memcpy(temp,typeV.files,typeV.size * sizeof(struct fileType));
    clearfileTypeArray();//deallocates old storage
    typeV.files = temp;//assign new memory
    typeV.capacity = sz;//update capacity
}

/*
* adds a mime type to dynamic array
*/
void addMimeType(const char* key,const char* val)
{
    if(typeV.size == 0)
        initV(8);//allocate raw memory to hold a minimum number of elements
    if(typeV.size == typeV.capacity)//expand if size reaches its capacity
        expand();
    typeV.files[typeV.size].type = key;
    typeV.files[typeV.size].name = val;
    ++typeV.size;//update size
}

/*
*   finds value of a mime type.
*   if not found the default value is returned
*/
const char* getMimeType(const char* key)
{
    int i = 0;
    const char* def = NULL;
    
    for(; i < typeV.size; ++i){
        if(strcasecmp(typeV.files[i].type,key) == 0){//found
            return typeV.files[i].name;
        }
        if(strcasecmp(typeV.files[i].type,"DEFAULT") == 0){//default value
            def = typeV.files[i].name;
        }
    }
    return def;//not found
}


