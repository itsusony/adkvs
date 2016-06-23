typedef struct {
    char *key;
    unsigned long long idx;
    UT_hash_handle hh; 
} K;

enum CMD_TYPE {
    CMD_NULL,
    CMD_UNKNOWN,
    CMD_QUIT,
    CMD_OK,
    CMD_ERR,
    CMD_GET,
    CMD_ECHO,
};

enum B_STATUS {
    B_EMPTY,
    B_SET,
    B_DEL,
};

typedef struct {
    char key[32];
    char val[2048];
    enum B_STATUS status;
} B;
