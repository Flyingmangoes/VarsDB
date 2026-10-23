#include "Fork.hpp"

bool daemonize(std::string log_path) {
    pid_t pid = fork();
    if (pid < 0) return false;
    if (pid > 0) _exit(EXIT_SUCCESS);

    if (setsid() < 0) return false;

    pid = fork();
    if (pid < 0) return false;
    if (pid > 0) _exit(EXIT_SUCCESS);

    umask(0);
    if (chdir("/") < 0) return false;

    int null_fd = open("/dev/null", O_RDWR);
    int log_fd  = open(log_path.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (null_fd < 0 || log_fd < 0) return false;

    dup2(null_fd, STDIN_FILENO);
    dup2(log_fd,  STDOUT_FILENO);
    dup2(log_fd,  STDERR_FILENO);
    close(null_fd);
    close(log_fd);

    return true;
}

void write_pidfile(const char* path) {
    std::ofstream f(path);
    f << getpid() << std::endl;
}