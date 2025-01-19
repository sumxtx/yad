#ifndef YAD_PROCESS_HPP
#define YAD_PROCESS_HPP

#include <filesystem>
#include <memory>
#include <sys/types.h>

namespace yad
{
    enum class process_state
    {
        stopped,
        running,
        exited,
        terminated
    };

    class process
    {
        public:
            ~process();
            static std::unique_ptr<process> launch(std::filessytem::path path);
            static std::unique_ptr<process> attach(pid_t pid);

            void resume();
            /*?*/ wait_on_signal();

            process() = delete;
            process(const process&) = delete;
            process& operator=(const process&) = delete;

            pid_t pid() const {return pid_;}
            process_state state() const { return state_; }

        private:
            process(pid_t pid, bool terminate_on_end)
                : pid_(pid), terminate_on_end_(terminate_on_end) {}

            pid_t pid_ = 0;
            bool terminate_on_end_ = true;
            
            process_state state_ = process_state::stopped;
    };
}

#endif
