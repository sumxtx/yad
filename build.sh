mkdir build && cd build
cmake ../
cmake --build .

# Add capabilities to yad to trace non-child processes 
# cd tools && setcap CAP_SYS_PTRACE=+eip yad.

# or globally allow PTRACE_ATTACH
# sudo echo 0 > /proc/sys/kernel/yama/ptrace_scope

# .{YAD_ROOT}/build/tools/yad # bin location

