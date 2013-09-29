//
//  lua_xhttp.c
//  xhttpd
//
//  Created by Peng Yunchou on 13-9-16.
//  Copyright (c) 2013年 Peng Yunchou. All rights reserved.
//

#include <stdio.h>
#include "lua_xhttp.h"
#include "lua_multipart_parser.h"

LUALIB_API int luaopen_xhttp_version(lua_State *L){
    lua_pushstring(L, "xhttp version 1.0 by pengyunchou.");
    return 1;
}

LUALIB_API int luaopen_xhttp (lua_State *L){
    lua_newtable(L);
    lua_pushstring(L, "multipart_parser");
    luaL_requiref(L, "xhttp.multipart_parser", luaopen_multipartparser, 0);
    lua_settable(L, -3);
    
    
    return 1;
}