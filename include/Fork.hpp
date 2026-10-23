#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <string>
#include <fstream>

bool daemonize(std::string log_path = "/tmp/varsdb.log");    
void write_pidfile(const char* path);