//
//  lua_xhttp.h
//  xhttpd
//
//  Created by Peng Yunchou on 13-9-16.
//  Copyright (c) 2013年 Peng Yunchou. All rights reserved.
//

#ifndef xhttpd_lua_xhttp_h
#define xhttpd_lua_xhttp_h

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

LUALIB_API int luaopen_xhttp (lua_State *L);

#endif
