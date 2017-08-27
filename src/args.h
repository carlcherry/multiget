#ifndef __multiget_args_include__
#define __multiget_args_include__

#include <iostream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <string>
using std::string;

/*! \brief Command line argument parser
 *
 * Read, parse and validate all the arguments from the command line using boost::program_options.
 *
 * NOTE: \"bytes\" takes precedence over chunks and size.  The following logic is used.
 *  1. If bytes is included then chunk size will be calculated: size = bytes/chunks
 *     and the remainder will be downloaded in the last chunk
 *  2. If bytes is not specified then bytes = chunks * size
 */
class Args {
public:
    Args();
    virtual ~Args() {}
    
    /**
     *   @brief  Perform the actual parse of the command line.
     *
     *   @param  argc From main
     *   @param  argv From main
     *
     *   @return true if parsed and validated, false otherwise
     */
    bool parseCommandLine(int argc, char* argv[]);
    
    /**
     *   @brief  Print usage information for this program
     *
     *
     *   @return void
     */
    void usage();
    
    /**
     *   @brief  Get the output filename (-o argument)
     *
     *   @return output filename
     */
    const std::string& getOutputFile() {
        return output_file_name_;
    }
    /**
     *   @brief  Get the server name parsed from the URL
     *
     *   @return server part of url
     */
    const std::string& getServer() {
        return server_;
    }
    /**
     *   @brief  Get the port parsed from the URL
     *
     *   If a port is included it will be return, otherwise port "80" is returned
     *   @return port part of url
     */
    const std::string& getPort() {
        return port_;
    }
    /**
     *   @brief  Get the path parsed from the URL
     *
     *   @return path part of url
     */
    const std::string& getPath() {
        return path_;
    }
    /**
     *   @brief  Get the entire URL passed in on the command line
     *
     *   @return url
     */
    const std::string& getURL() {
        return url_;
    }
    /**
     *   @brief  Get value for parallem mode (-p argument)
     *
     *   @return true if in parallel mode, false if in serial mode
     */
    bool downloadInParallel() {
        return parallel_download_;
    }
    /**
     *   @brief  Get the number of chunks to break the request into (-c argument)
     *
     *   @return The number of chunks used to download the file
     */
    int getChunkCount() {
        return chunk_count_;
    }
    /**
     *   @brief  Get the size of each chunk (-s argument)
     *
     *   @return chunk size in bytes
     */
    int getChunkSize() {
        return chunk_size_;
    }
    /**
     *   @brief  Get the total number of bytes to download (-b argument)
     *
     *   @return Total number of bytes to request
     */
    int getTotalSize() {
        return total_size_;
    }
    /**
     *   @brief  Get the number of threads to use in parallel mode (-t argument)
     *
     *   @return thread count
     */
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

