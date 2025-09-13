Import("env")
env.Execute("patch --reject-file=- .pio/libdeps/digispark-tiny/AHTxx/src/AHTxx.cpp < scripts/AHTxx.cpp.patch || true")
