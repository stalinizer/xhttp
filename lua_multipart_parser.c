//
//  lua_multipart_parser.c
//  xhttpd
//
//  Created by Peng Yunchou on 13-9-16.
/*
Copyright (c) <2014>, skysent
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software
   must display the following acknowledgement:
   This product includes software developed by the <organization>.
4. Neither the name of the <organization> nor the
   names of its contributors may be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY skysent ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include "lua_multipart_parser.h"
#include "multipart_parser.h"
#include <string.h>
#include "xlualib.h"
//#define XMULTIPART_PARSER_DEBUG
char* gettempfilename(){
    return tmpnam (NULL);
}


struct xmultipart_parser_ctx {
    struct{
        int on_header_field;
        int on_header_value;
        int on_part_data;
        
        int on_part_data_begin;
        int on_headers_complete;
        int on_part_data_end;
        int on_body_end;
    }luacbs;
    struct multipart_parser_settings settings;
    struct multipart_parser *mparser;
    char boundary[128];
    lua_State *L;
};
int call_lua_cb(struct xmultipart_parser_ctx *ctx,int lcb,const char *at,size_t length){

    if (lcb==-1) {
        return 0;
    }
    lua_rawgeti(ctx->L, LUA_REGISTRYINDEX, lcb);
    lua_pushlstring(ctx->L, at, length);
    lua_call(ctx->L, 1, 0);
    return 0;
}
int notifi_lua_cb(struct xmultipart_parser_ctx *ctx,int lcb){
    if (lcb==-1) {
        return 0;
    }
    lua_rawgeti(ctx->L, LUA_REGISTRYINDEX, lcb);
    lua_call(ctx->L, 0, 0);
    return 0;
}
int on_header_field_cb(multipart_parser* parser, const char *at, size_t length){
    struct xmultipart_parser_ctx *ctx=multipart_parser_get_data(parser);
    return call_lua_cb(ctx, ctx->luacbs.on_header_field, at, length);
}
int on_header_value_cb(multipart_parser* parser, const char *at, size_t length){
    struct xmultipart_parser_ctx *ctx=multipart_parser_get_data(parser);
    return call_lua_cb(ctx, ctx->luacbs.on_header_value, at, length);
}
int on_part_data_cb(multipart_parser* parser, const char *at, size_t length){
    struct xmultipart_parser_ctx *ctx=multipart_parser_get_data(parser);
    return call_lua_cb(ctx, ctx->luacbs.on_part_data, at, length);
}

int on_part_data_begin_cb (multipart_parser* parser){
    struct xmultipart_parser_ctx *ctx=multipart_parser_get_data(parser);
    return notifi_lua_cb(ctx,ctx->luacbs.on_part_data_begin);
}
int on_headers_complete_cb (multipart_parser* parser){
    struct xmultipart_parser_ctx *ctx=multipart_parser_get_data(parser);
    return notifi_lua_cb(ctx,ctx->luacbs.on_headers_complete);
}
int on_part_data_end_cb (multipart_parser* parser){
    struct xmultipart_parser_ctx *ctx=multipart_parser_get_data(parser);
    return notifi_lua_cb(ctx,ctx->luacbs.on_part_data_end);
}
int on_body_end_cb (multipart_parser* parser){
    struct xmultipart_parser_ctx *ctx=multipart_parser_get_data(parser);
    return notifi_lua_cb(ctx,ctx->luacbs.on_body_end);
}

void xmultipart_parser_dumy_init_settings(struct xmultipart_parser_ctx* ctx){
    struct multipart_parser_settings* settings=&ctx->settings;
    settings->on_header_field=on_header_field_cb;
    settings->on_header_value=on_header_value_cb;
    settings->on_part_data=on_part_data_cb;
    
    settings->on_part_data_begin=on_part_data_begin_cb;
    settings->on_headers_complete=on_headers_complete_cb;
    settings->on_part_data_end=on_part_data_end_cb;
    settings->on_body_end=on_body_end_cb;
}
LUA_API int luaopen_multipartparser_destroy(lua_State *L){
    struct xmultipart_parser_ctx *ctx=lua_touserdata(L, 1);
    luaL_argcheck(L, ctx!=NULL, 1, "struct xmultipart_parser_ctx expected");
    if (ctx->luacbs.on_body_end!=-1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ctx->luacbs.on_body_end);
    }
    if (ctx->luacbs.on_header_field!=-1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ctx->luacbs.on_header_field);
    }
    if (ctx->luacbs.on_header_value!=-1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ctx->luacbs.on_header_value);
    }
    if (ctx->luacbs.on_headers_complete!=-1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ctx->luacbs.on_headers_complete);
    }
    if (ctx->luacbs.on_part_data!=-1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ctx->luacbs.on_part_data);
    }
    if (ctx->luacbs.on_part_data_begin!=-1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ctx->luacbs.on_part_data_begin);
    }
    if (ctx->luacbs.on_part_data_end!=-1) {
        luaL_unref(L, LUA_REGISTRYINDEX, ctx->luacbs.on_part_data_end);
    }
    
    free(ctx);
    return 1;
}

LUA_API int luaopen_multipartparser_new(lua_State *L){
    if (lua_istable(L, 1)) {
        struct xmultipart_parser_ctx *ctx=malloc(sizeof(struct xmultipart_parser_ctx));
        memset(ctx, 0x0, sizeof(struct xmultipart_parser_ctx));
        ctx->luacbs.on_body_end=-1;
        ctx->luacbs.on_header_field=-1;
        ctx->luacbs.on_header_value=-1;
        ctx->luacbs.on_headers_complete=-1;
        ctx->luacbs.on_part_data=-1;
        ctx->luacbs.on_part_data_begin=-1;
        ctx->luacbs.on_part_data_end=-1;
        
        
        const char *boundary=NULL;
        lua_pushstring(L, "boundary");
        lua_gettable(L, -2);
        boundary=luaL_checkstring(L, -1);
        lua_pop(L, 1);
        luaL_argcheck(L, boundary!=NULL, 1, "boundary must not be null.");
        memcpy(ctx->boundary, boundary, strlen(boundary));
        
        lua_pushstring(L, "on_header_field");
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            ctx->luacbs.on_header_field=luaL_ref(L, LUA_REGISTRYINDEX);
#ifdef XMULTIPART_PARSER_DEBUG
            printf("on_header_field->%d\n",ctx->luacbs.on_header_field);
#endif
        }else{
            lua_pop(L, 1);
        }

        lua_pushstring(L, "on_header_value");
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            ctx->luacbs.on_header_value=luaL_ref(L, LUA_REGISTRYINDEX);
#ifdef XMULTIPART_PARSER_DEBUG
            printf("on_header_value->%d\n",ctx->luacbs.on_header_value);
#endif
        }else{
            
            lua_pop(L, 1);
        }
        lua_pushstring(L, "on_part_data");
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            ctx->luacbs.on_part_data=luaL_ref(L, LUA_REGISTRYINDEX);
#ifdef XMULTIPART_PARSER_DEBUG
            printf("on_part_data->%d\n",ctx->luacbs.on_part_data);
#endif
        }else{
            lua_pop(L, 1);
        }
        
        
        lua_pushstring(L, "on_part_data_begin");
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            ctx->luacbs.on_part_data_begin=luaL_ref(L, LUA_REGISTRYINDEX);
#ifdef XMULTIPART_PARSER_DEBUG
            printf("on_part_data_begin->%d\n",ctx->luacbs.on_part_data_begin);
#endif
        }else{
            lua_pop(L, 1);
        }
        
        
        lua_pushstring(L, "on_headers_complete");
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            ctx->luacbs.on_headers_complete=luaL_ref(L, LUA_REGISTRYINDEX);
#ifdef XMULTIPART_PARSER_DEBUG
            printf("on_headers_complete->%d\n",ctx->luacbs.on_headers_complete);
#endif
        }else{
            lua_pop(L, 1);
        }
        
        
        lua_pushstring(L, "on_part_data_end");
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            ctx->luacbs.on_part_data_end=luaL_ref(L, LUA_REGISTRYINDEX);
#ifdef XMULTIPART_PARSER_DEBUG
            printf("on_part_data_end->%d\n",ctx->luacbs.on_part_data_end);
#endif
        }else{
            lua_pop(L, 1);
        }
        
        
        lua_pushstring(L, "on_body_end");
        lua_gettable(L, -2);
        if (lua_isfunction(L, -1)) {
            ctx->luacbs.on_body_end=luaL_ref(L, LUA_REGISTRYINDEX);
#ifdef XMULTIPART_PARSER_DEBUG
            printf("on_body_end->%d\n",ctx->luacbs.on_body_end);
#endif
        }else{
            lua_pop(L, 1);
        }
#ifdef XMULTIPART_PARSER_DEBUG
        {
            printf("lua callbacks-----------------------\n");
            printf("%d\n",ctx->luacbs.on_body_end);
            printf("%d\n",ctx->luacbs.on_header_field);
            printf("%d\n",ctx->luacbs.on_header_value);
            printf("%d\n",ctx->luacbs.on_headers_complete);
            printf("%d\n",ctx->luacbs.on_part_data);
            printf("%d\n",ctx->luacbs.on_part_data_begin);
            printf("%d\n",ctx->luacbs.on_part_data_end);
            printf("end lua callbacks-----------------------\n");
        }
#endif
        ctx->L=L;
        xmultipart_parser_dumy_init_settings(ctx);
        
        ctx->mparser=multipart_parser_init(boundary, &ctx->settings);
        multipart_parser_set_data(ctx->mparser, ctx);
        lua_pushlightuserdata(L, ctx);
        return 1;
    }else{
        return luaL_error(L, "luaopen_multipartparser_new=>parameter #1 to new need a table.");
    }
}

LUA_API int xmultipart_parser_tmpfile(lua_State *L){
    char *tempname=gettempfilename();
    lua_pushstring(L, tempname);
    return 1;
}
LUA_API int xmultipart_parser_execute(lua_State *L){
#ifdef XMULTIPART_PARSER_DEBUG
    printf("execute....\n");
#endif
    size_t s;
    struct xmultipart_parser_ctx *ctx=lua_touserdata(L, 1);
    luaL_argcheck(L, ctx!=NULL, 1, "xmultipart_parser_ctx expected");
    const char *data=luaL_checklstring(L, 2, &s);
    size_t pl=multipart_parser_execute(ctx->mparser, data, s);
    lua_pushnumber(L, pl);
    return 1;
}
LUA_API int luaopen_multipartparser(lua_State *L){
    luaL_Reg multipartparser_methods[]={
        {"new",luaopen_multipartparser_new},
        {"tmpfile",xmultipart_parser_tmpfile},
        {"execute",xmultipart_parser_execute},
        {"close",luaopen_multipartparser_destroy},
        {NULL,NULL}
    };
    luaL_newlib(L, multipartparser_methods);
    return 1;
}
