#define PORT 1985

struct event_base *base;
struct evconnlistener *listener;

unsigned long long storage_length = 10000*100;
const char*        storage_path = "/var/data/adkvs_mmap";
int mmap_fd;       
B*  mmap_ptr;

unsigned long long storage_index = 0;

K* cached_keys=NULL;
void cached_keys_append(const char* key, unsigned long long i){ 
    if (!key || !strlen(key))return;
    K* k = malloc(sizeof(K));
    k->key = strdup(key);
    k->idx = i;
    HASH_ADD_KEYPTR(hh,cached_keys,k->key,strlen(k->key),k);
}
unsigned long long cached_keys_find(const char* key) {
    if (!key || !strlen(key))return -1; 
    K* k = NULL;
    HASH_FIND_STR(cached_keys, key, k); 
    return k ? k->idx : -1; 
}
int cached_keys_remove(const char* key) {
    if (!key || !strlen(key))return 0;
    K* k = NULL;
    HASH_FIND_STR(cached_keys, key, k); 
    if (k) {
        free(k->key);
        HASH_DEL(cached_keys,k);
        return 1;
    }   
    return 0;
}
