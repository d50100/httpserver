cmake_minimum_required(VERSION 3.13)

project("httpserver")

include_directories(".")
include_directories("include")

include_directories("lib/kcp")

add_executable("server" 
	src/kcp-server.c 
	src/utils.c
	lib/kcp/ikcp.c
	)

add_executable("client" 
	src/kcp-client.c 
	src/utils.c
	lib/kcp/ikcp.c
	)
