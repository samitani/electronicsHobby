Import("env")
env.Execute("patch .pio/libdeps/digispark-tiny/AHTxx/src/AHTxx.cpp < scripts/AHTxx.cpp.patch")
