#include "cachelab.h"
#include<getopt.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<stdio.h>

int hit_count = 0;
int miss_count = 0;
int eviction_count = 0;
int verbose = 0;
char* t = NULL;

void show_help(){
    printf("使用方法： ./csim-ref [-h] -s <s> -E <E> -b <b> -t <tracefile>");
    printf("可选参数\n");
    printf("    -h              显示帮助信息并退出\n");
    printf("    -v              显示详细输出\n");
    printf("必选参数\n");
    printf("    -s <s>          组索引位的位数\n");
    printf("    -E <E>          相联度\n");
    printf("    -b <b>          块偏移位的位数\n");
    printf("    -t <tracefile>  待重放的 valgrind 跟踪文件名称\n");
}
typedef struct cacheLine cacheLine;
// 多条双向链表
// 头节点结构为：节点数n(n <= E + 1)、指向后一个节点的指针next。
typedef struct{
    int len;
    cacheLine* tail;
    cacheLine* next;
}listHead;

// 节点结构为：标记位tag、指向前一个节点的指针pre、指向后一个节点的指针next。
struct cacheLine{
    int tag;
    cacheLine* pre;
    cacheLine* next;
};
// 用一个指针数组记录所有头节点地址，数组下标即为组号(从s得来)。
// cache命中：将命中节点移动到链表头部
// cache未命中：创建一个新节点。若链表未满，插入到链表头部；若链表已满，删除链表尾部节点，再插入到链表头部。

// 需要实现的函数：

// 初始化头节点及指针数组
listHead* initLists(int setSize){
    listHead* cache = (listHead*)malloc(setSize * sizeof(listHead));
    if(cache == NULL) return NULL;
    for(int i = 0; i < setSize; i++){
        cache[i].len = 0;
        cache[i].tail = NULL;
        cache[i].next = NULL;
    }
    return cache;
}

// 查询cache
cacheLine* findCache(int setIndex, int curTag, listHead* cache){
    cacheLine* cur = cache[setIndex].next;
    while(cur != NULL){
        if(cur->tag == curTag){
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}
// 命中处理
void whenHit(int setIndex, cacheLine* cur, listHead* cache){
    hit_count++;
    if(verbose) printf("Hit");
    // 若在链表首部直接返回
    if(cache[setIndex].next == cur) return;
    // 若在链表尾部
    if(cache[setIndex].tail == cur){
        cur->pre->next = NULL;
        cache[setIndex].tail = cur->pre;
    }else{
        cur->pre->next = cur->next;
        cur->next->pre = cur->pre;
    }

    cur->pre = NULL;
    cur->next = cache[setIndex].next;
    cache[setIndex].next = cur;
    if(cur->next != NULL){
        cur->next->pre = cur;
    }
}

// 未命中处理
void whenMiss(int setIndex, int curTag, int maxLines, listHead* cache){
    miss_count++;
    if(verbose) printf("Miss");
    // 创建一个新cache行
    cacheLine* newLine = (cacheLine*)malloc(sizeof(cacheLine));
    newLine->tag = curTag;
    newLine->pre = NULL;
    newLine->next = NULL;

    // 若cache满
    if(cache[setIndex].len == maxLines){
        eviction_count++;
        if(verbose) printf("Eviction");
        cacheLine* lastLine = cache[setIndex].tail;
        if(cache[setIndex].len == 1){
            cache[setIndex].next = NULL;
            cache[setIndex].tail = NULL;
        }else{
            cache[setIndex].tail = lastLine->pre;
            cache[setIndex].tail->next = NULL;
        }
        free(lastLine);
        lastLine = NULL;
        cache[setIndex].len--;
    }

    newLine->next  = cache[setIndex].next;
    if(cache[setIndex].next != NULL){
        cache[setIndex].next->pre = newLine;
    }else{
        cache[setIndex].tail = newLine;
    }
    cache[setIndex].next = newLine;
    cache[setIndex].len++;
    if(verbose) printf("\n");
}

void updateInfo(int setIndex, int curTag, int maxLines, listHead* cache){
    cacheLine* cur = findCache(setIndex, curTag, cache);
    if(cur != NULL){
        whenHit(setIndex, cur, cache);
    }else{
        whenMiss(setIndex, curTag, maxLines, cache);
    }
}

// 读取文件
void getTrace(int s, int b, int maxLines, char* t, listHead* cache){
    FILE* pFile;
    pFile = fopen(t, "r");
    if(pFile == NULL){
        exit(-1);
    }
    char opt;
    unsigned long long address;
    int size;
    while(fscanf(pFile, " %c %llx,%d", &opt, &address, &size) > 0){
        int curTag = (int)address >> (s + b);
        unsigned int setMask = (1 << s) - 1;
        int setIndex = (int)((address >> b) & setMask);
        switch(opt){
            case 'M':
                updateInfo(setIndex, curTag, maxLines, cache);
                updateInfo(setIndex, curTag, maxLines, cache);
                break;
            case 'L':
                updateInfo(setIndex, curTag, maxLines, cache);
                break;
            case 'S':
                updateInfo(setIndex, curTag, maxLines, cache);
                break;
            default:
                break;
        }
    }
    fclose(pFile);
}

void freeCache(int setSize, listHead* cache){
    for(int i = 0; i < setSize; i++){
        cacheLine* cur = cache[i].next;
        while(cur){
            cacheLine* next = cur->next;
            free(cur);
            cur = next;
        }
    }
    free(cache);
}

int main(int argc, char* argv[]){
    char opt;
    int s = -1, E = -1, b = -1;
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch(opt){
            case 'h':
                show_help();
                exit(0);
            case 'v':
                verbose = 1;
                break;
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                t = optarg;
                break;
            default:
                show_help();
                exit(-1);
        }
    }
    if(s == -1 || E == -1 || b == -1 || t == NULL){
        fprintf(stderr, "错误：缺少必选参数！\n");
        show_help();
        exit(-1);
    }

    int maxLines = E;
    int setSize = 1 << s;
    listHead* cache = initLists(setSize);
    getTrace(s, b, maxLines, t, cache);
    printSummary(hit_count, miss_count, eviction_count);
    freeCache(setSize, cache);
    return 0;
}