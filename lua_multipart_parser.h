//
//  lua_multipart_parser.h
//  xhttpd
//
//  Created by Peng Yunchou on 13-9-16.
//  Copyright (c) 2013年 Peng Yunchou. All rights reserved.
//

#ifndef xhttpd_lua_multipart_parser_h
#define xhttpd_lua_multipart_parser_h
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
LUA_API int luaopen_multipartparser(lua_State *L);

#endif
