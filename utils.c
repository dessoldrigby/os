#include "utils.h"
#include <stdarg.h>
#include <string.h>
#include <unistd.h>



#ifdef _USTRSTACK

// ----------------------------------------------------------------------------------------
// string stack impl

strnode_t* create_strnode(char* value, int size) 
{
    strnode_t* res = (strnode_t*)malloc(sizeof(strnode_t));

    int strlen_sz = strlen(value); 

    if(size <= 0) size = strlen_sz;
    if(size == 0 && strlen_sz <= 0) return NULL;
    

    res->str = (char*)malloc(sizeof(char) * (size + 1));
    if(!res->str) ERR("Cannot allocate memmory");

    strncpy(res->str, value, size);
    res->size = size + 1;
    res->str[res->size - 1] = '\0';
    res->next = NULL;

    return res;
}

void dfstrnode(strnode_t* node) 
{
    if (NULL == node) return;

    dfstrnode(node->next);
    free(node->str);
    free(node);
}



strstack_t strstack_create() 
{
    strstack_t res = { 0, NULL };

    return res;
}

strstack_t strstack_create_from_array(char* arr[], int arr_size)
{
    strstack_t res = strstack_create();

    for(int i = 0; i < arr_size; i++) {
        strstack_push(&res, arr[i], -1);
    }

    return res;
}

void strstack_clear(strstack_t* stack) 
{
    dfstrnode(stack->head);
    stack->size = 0;
}

bool strstack_is_empty(strstack_t* stack) 
{
    return 0 == stack->size || NULL == stack->head;
}

void strstack_push(strstack_t* stack, char* value, int size) 
{
    strnode_t* newNode = create_strnode(value, size);

    if(NULL == newNode) return;

    stack->size++;

    if (strstack_is_empty(stack)) {
        stack->head = newNode;
        return;
    }

    newNode->next = stack->head;
    stack->head = newNode;
}

char* strstack_pop(strstack_t* stack, int* out_size) 
{
    if (NULL != out_size) *out_size = 0;
    if (strstack_is_empty(stack) || NULL == stack->head->str) return NULL;

    stack->size--;

    char* res = stack->head->str;
    if (NULL != out_size) *out_size = stack->head->size;
    strnode_t* n = stack->head->next;
    free(stack->head);

    stack->head = n;

    return res;
}

void strstack_for(strstack_t* stack, void (*fun)(char*, int)) 
{
    if (strstack_is_empty(stack)) return;

    strnode_t* ptr = stack->head;

    do {
        fun(ptr->str, ptr->size);
    } while (NULL != (ptr = ptr->next));
}

// end impl
// ----------------------------------------------------------------------------------------

#endif



#ifdef _USTR

void get_string(char* str, int size, FILE* stream)
{
    // fgets writes the \n symbol after the finishing of an input operation which leads
    // to a problem with recognizing correctly a file name
    fgets(str, size, stream);
    int len = strlen(str);
    str[len] = ' ';
    str[len - 1] = '\0';
}

void split_string(char* str, char* delim, strstack_t* out_strings) {
    if(NULL == out_strings) {
        *out_strings = strstack_create();
    }
    
    char *sp;
    char *token = strtok_r(str, delim, &sp);

    while (NULL != token) {
        strstack_push(out_strings, token, -1);

        token = strtok_r(NULL, delim, &sp);
    }
}

#endif



#ifdef _UFILES

int bulk_close(int fd)
{
    return TEMP_FAILURE_RETRY(close(fd));
}

ssize_t bulk_read(int fd, char* buf, size_t count) 
{
    ssize_t c, len = 0;

    do {
        c = TEMP_FAILURE_RETRY(read(fd, buf, count));
        if (c < 0) return c;
        if(0 == c) return len; // \EOF

        buf += c;
        len += c;
        count -= c;
    } while (count > 0);

    return len;
}

ssize_t bulk_write(int fd, char* buf, size_t count) 
{
    ssize_t c, len = 0;

    do {
        c = TEMP_FAILURE_RETRY(write(fd, buf, count));
        if (c < 0) return c;

        buf += c;
        len += c;
        count -= c;
    } while (count > 0);

    return len;
}

#endif

#ifdef _USIG

int set_sig_handler(void (*handler)(int), int sig_no)
{
    struct sigaction act;
    memset(&act, 0, sizeof(struct sigaction));
    act.sa_handler = handler;

    if(-1 == sigaction(sig_no, &act, NULL))
        return -1;

    return 0;
}

int set_sig_action(void (*handler)(int, siginfo_t*, void*), int sig_no)
{
    struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction = handler;
	act.sa_flags = SA_SIGINFO;
	if (-1 == sigaction(sig_no, &act, NULL))
		return -1;

    return 0;
}

#endif