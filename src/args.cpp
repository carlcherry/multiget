#include "args.h"
#include <boost/regex.hpp>

Args::Args()
: desc_("Usage: ./multiget [OPTIONS] url")
, output_file_name_("multiget.out")
, parallel_download_(false)
, chunk_count_(4)
, chunk_size_(1024*1024) // 1 MiB
, total_size_(1024*1024*4) // 4 MiB
, thread_count_(1)
{
}

void Args::usage()
{
    std::cout << desc_ << std::endl;
    std::cout <<
    "NOTE: \"bytes\" takes precedence over chunks and size.  The following logic is used." << std::endl <<
    "  1. If bytes is included then chunk size will be calculated: size = bytes/chunks" << std::endl <<
    "     and the remainder will be downloaded in the last chunk" <<  std::endl <<
    "  2. If bytes is not specified then bytes = chunks * size" << std::endl;
}

bool Args::parseCommandLine(int argc, char* argv[])
{
    desc_.add_options()
        ("help,h", "produce help message")
        ("outputfile,o", po::value<std::string>(&output_file_name_), "Name of the downloaded file (default is multiget.out)")
        ("parallel,p", "Download the chunks simultaneously")
        ("threads,t", po::value<int>(&thread_count_), "Number of threads to use during downlaod (default is 1)")
        ("size,s", po::value<int>(&chunk_size_), "Chunk size for downloading the file (default is 1 MiB)")
        ("chunks,c", po::value<int>(&chunk_count_), "Number of chunks to downlaod (default is 4)")
        ("bytes,b", po::value<int>(&total_size_), "Total number of bytes to download (default is 4 MiB)");
    
    // We wont want to force someone to use --url or -u on the command line.  So we need to
    // create a hidden/postional option that is at the end of the command line
    po::options_description hidden_option;
    hidden_option.add_options() ("url", po::value<std::string>(&url_), "URL to fetch");
    
    po::positional_options_description positional_options;
    positional_options.add("url", -1);
    
    // Now combine the options and parse them
    po::options_description all_options;
    all_options.add(desc_);
    all_options.add(hidden_option);
    
    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(all_options).positional(positional_options).run(), vm);
    
    if (vm.count("help")) {
        return false; // This will trigger a call to "usage" and then quit
    }
    if (!vm.count("url")) {
        std::cout << "ERROR - missing command line argument: url" << std::endl;
        return false;
    }
    
    // We now have validate the only required parameter - so call notify to fill in all the values
    po::notify(vm);
    
    if (vm.count("parallel")) {
        parallel_download_ = true;
    }
    
    return validateParameters(vm);
}

bool Args::validateParameters(po::variables_map& vm)
{
    // Parse the URL into server, port, and path
    if (!parseURL()) {
        return false;
    }
    
    // Validate some parameters
    if (total_size_ == 0) {
        std::cout << "\"bytes\" cannot be set to 0" << std::endl;
        return false;
    }
    if (chunk_size_ == 0) {
        std::cout << "\"size\" cannot be set to 0" << std::endl;
        return false;
    }
    if (chunk_count_ == 0) {
        std::cout << "\"chunks\" cannot be set to 0" << std::endl;
        return false;
    }
    
    if (vm.count("bytes")) {
        // The user specified the total number of bytes which takes precedence over the other
        // parameters.  It is up to the user of this class to realize that chunk_size_ * chunk_count_
        // is not equal to total_size_
        chunk_size_  = total_size_ / chunk_count_;
    } else {
        // Total size not specified - so calculate it based on the the chunk count and chunk size
        total_size_ = chunk_size_ * chunk_count_;
    }
    return true;
}

bool Args::parseURL()
{
    // Parse the URL into server, port, and path
    boost::regex pattern("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)");
    boost::cmatch url_parts;
    if(regex_match(url_.c_str(), url_parts, pattern))
    {
        server_ = std::string(url_parts[2].first, url_parts[2].second);
        port_ = std::string(url_parts[3].first, url_parts[3].second);
        path_ = std::string(url_parts[4].first, url_parts[4].second);
        if (port_.length() == 0) {
            port_ = "http";
        }
        if (server_.length() == 0) {
            std::cout << "Unable to parse server from the url: " << url_ << std::endl;
            return false;
        }
        if (path_.length() == 0) {
            std::cout << "Unable to parse file path from the url: " << url_ << std::endl;
            return false;
        }
        return true;
    } else {
        std::cout << "Unable to parse the URL: " << url_ << std::endl;
        return false;
    }
}
