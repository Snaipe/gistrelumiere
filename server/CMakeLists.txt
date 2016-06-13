set (SOURCES
    date.c
    date.h
    httpsrv.c
    httpsrv.h
    panic.c
    panic.h
    precomp.h
    print.c
    print.h
    sampler.c
    sampler.h
    server.c
)

add_executable(lumiere ${SOURCES})
target_link_libraries(lumiere httpapi)
