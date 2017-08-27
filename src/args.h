#ifndef __multiget_args_include__
#define __multiget_args_include__

#include <iostream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <string>
using std::string;

class Args {
public:
    Args();
    virtual ~Args() {}
    
    bool parseCommandLine(int argc, char* argv[]);
    void usage();
    
    const std::string& getOutputFile() {
        return output_file_name_;
    }
    const std::string& getServer() {
        return server_;
    }
    const std::string& getPort() {
        return port_;
    }
    const std::string& getPath() {
        return path_;
    }
    const std::string& getURL() {
        return url_;
    }
    bool downloadInParallel() {
        return parallel_download_;
    }
    int getChunkCount() {
        return chunk_count_;
    }
    int getChunkSize() {
        return chunk_size_;
    }
    int getTotalSize() {
        return total_size_;
    }
    int getThreadCount() {
        return thread_count_;
    }
private:
    bool parseURL();
    bool validateParameters(po::variables_map& vm);
    
    po::options_description desc_;
    std::string output_file_name_;
    std::string url_;
    std::string server_;
    std::string port_;
    std::string path_;
    bool parallel_download_;
    int chunk_count_;
    int chunk_size_;
    int total_size_;
    int thread_count_;
};

#endif // __multiget_args_h__

