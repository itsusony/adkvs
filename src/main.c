#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "uthash.h"

#include "type.c"
#include "global.c"
#include "mmap.c"
#include "storage.c"

static enum CMD_TYPE parse_cmd(char* str, size_t len, char** output_val) {
    if (len > 0) {
        if (strcmp(str,"q")==0)return CMD_QUIT;
        char* dup_str = strdup(str);
        char cmd[16]="",key[32]="",val[2048]="";
        char *saveptr=NULL;
        char *token = strtok_r(dup_str," ",&saveptr);
        int token_len = token ? strlen(token) : 0;
        // required
        if (token_len == 0 || token_len > 16) { free(dup_str);return CMD_ERR; }
        memcpy(cmd,token,token_len);
        // optional
        token = strtok_r(NULL," ",&saveptr);
        token_len = token ? strlen(token) : 0;
        if (token_len && token_len <= 32) memcpy(key,token,token_len);
        // optional
        token = strtok_r(NULL," ",&saveptr);
        token_len = token ? strlen(token) : 0;
        if (token_len) {
            if (token_len <= 2048) {
                memcpy(val,token,token_len);
            } else {
                free(dup_str);return CMD_ERR;
            }
        }
        free(dup_str);

        if (strncmp(cmd,"set",3)==0) {
            if (strlen(key)==0)return CMD_ERR;
            storage_set(key,val);
            // todo check mmap limit
            return CMD_OK;
        } else if (strncmp(cmd,"get",3)==0) {
            if (strlen(key)==0)return CMD_ERR;
            char* val = storage_get(key);
            *output_val = val;
            return val == NULL ? CMD_NULL : CMD_GET;
        } else if (strncmp(cmd,"ping",4)==0) {
            *output_val = "pong";
            return CMD_ECHO;
        }
    }
    return CMD_UNKNOWN;
}

static void echo_read_cb(struct bufferevent *bev, void *ctx) {
    struct evbuffer *input = bufferevent_get_input(bev);
    size_t str_len = 0;
    char* str_input = evbuffer_readln(input,&str_len,EVBUFFER_EOL_ANY);

    char* val = NULL;
    enum CMD_TYPE cmdtype = parse_cmd(str_input, str_len, &val);
    if (cmdtype == CMD_QUIT){
        bufferevent_free(bev);
        return;
    }
    struct evbuffer *ret = evbuffer_new();
    switch (cmdtype) {
        case CMD_GET:
        case CMD_ECHO:
            evbuffer_add(ret,val,strlen(val));
            break;
        case CMD_ERR:
            evbuffer_add(ret,"(ERR)",5);
            break;
        case CMD_OK:
            evbuffer_add(ret,"(OK)",4);
            break;
        case CMD_NULL:
            evbuffer_add(ret,"(NULL)",6);break;
        default:
            evbuffer_add(ret,"(UNKNOWN)",9);break;
    }
    evbuffer_add(ret,"\n",1);
    evbuffer_add_buffer(bufferevent_get_output(bev), ret);
}

static void echo_event_cb(struct bufferevent *bev, short events, void *ctx) {
    if (events & BEV_EVENT_ERROR)perror("Error from bufferevent");
    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))bufferevent_free(bev);
}

static void accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
    bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));
    event_base_loopexit(base, NULL);
}

int main(int argc, char **argv){
    base = event_base_new();
    storage_prepare();

    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(0);
    sin.sin_port = htons(PORT);

    listener = evconnlistener_new_bind(base, accept_conn_cb, NULL, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1, (struct sockaddr*)&sin, sizeof(sin));
    if (!listener) { perror("Couldn't create listener");return 1; }
    evconnlistener_set_error_cb(listener, accept_error_cb);
    event_base_dispatch(base);
    return 0;
}
