#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <memory>
using namespace std;

#include "httpget.h"
#include "args.h"

// Add a few helper functions to simply main
void concatenate_output(std::vector<HTTPGet*>& requests, const std::string& output_file_name);
void cleanup(std::vector<HTTPGet*>& request);
int  getFileSize(const std::string& filename);
void downloadInParallel(int num_threads, boost::asio::io_service *io_service);

int main(int argc, char* argv[])
{
    // Read in the command line arguments
    Args args;
    if (!args.parseCommandLine(argc, argv)) {
        args.usage();
        return EXIT_SUCCESS;
    }
    
    // Get all the parameters from the args class
    int num_chunks = args.getChunkCount(); // how many requests to make
    int total_bytes  = args.getTotalSize(); // the total number of bytes to get
    int chunk_size = args.getChunkSize(); // how many bytes to get on each request
    int remainder = total_bytes - (chunk_size * num_chunks);
    std::cout << "Getting a total of " << total_bytes << " in " << num_chunks << " chunks, of size " << chunk_size <<
        " with a remainder of " << remainder << std::endl;
    if (remainder > 0) {
        // The amount of bytes does not fit into an even number of chunks, so add
        // another chunk to get the last few bytes
        num_chunks++;
    }
    std::string output_file_name(args.getOutputFile()); // The name of the file to store the output
    
    std::cout << "Chunk size: " << chunk_size << ", num_chunks: " << num_chunks << std::endl;
    
    // Use a vector to store the HTTPGet request. We need to keep the requests in the proper order
    // so that we put the chunks back together in the correct order
    std::vector<HTTPGet*> requests;
    try {
        boost::asio::io_service io_service;
        for (int i = 0; i < num_chunks; i++) {
            // Figure out the start and end values for this chunk and assign a temp filename for the output
            int start_range = i * chunk_size;
            
            // The range starts indexing at 0 so we always need to minus 1 from the end of the range
            int end_range = start_range + chunk_size - 1;
            
            // If this is the last chunk we need to check if it is a full or partial chunk.
            if (i == (num_chunks - 1) && remainder > 0) {
                // Special case - adjust the end range
                end_range = start_range + remainder - 1;
            }
            
            std::stringstream output_file_name;
            output_file_name << "./tmpchunk" << i;
            
            // Create a HTTPGet object for this chunk of bytes and add to the end of the vector
            HTTPGet * request = new HTTPGet(io_service, args.getServer(), args.getPath(), args.getPort(),
                                            start_range, end_range, output_file_name.str().c_str());
            requests.push_back(request);
            if (!args.downloadInParallel()) {
                // Downloading in serial mode - so run the download for this request now
                io_service.run();
                io_service.reset();
            }
        }
        
        // If the user requested parallel downloading then we have not done any work yet
        // so kick off the io_service now.  There is a single io_service shared by the HTTPGet
        // objects (HTTPGet is implemented using all async methods).
        if (args.downloadInParallel()) {
            downloadInParallel(args.getThreadCount(), &io_service);
        }
    } catch (const std::exception& e) {
        std::cout << "Exception: " << e.what() << " - unable to download file" << std::endl;
    } catch (...) {
        std::cout << "Unknown exception - unable to download file" << std::endl;
    }
    
    // Take all the chunks and assemble them into a single file (and clean up the temp files)
    concatenate_output(requests, output_file_name);
    cleanup(requests);
    
    int file_size = getFileSize(output_file_name);
    if (file_size == total_bytes) {
        std::cout << std::endl << "Finished downloading " << args.getURL() << "  - to file " << output_file_name << std::endl;
    } else {
        std::cout << std::endl << "Size mismatch: expected: " << total_bytes << ", actual: " << file_size << std::endl;
    }
    
    return 0;
}

void concatenate_output(std::vector<HTTPGet*>& requests, const std::string& output_file_name)
{
    // Open up the output file
    std::cout << "Writing output file..." << std::endl;
    std::ofstream output_file(output_file_name, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
    // Concatenate all the output files into to the requested output file
    for (HTTPGet* request: requests) {
        // Get the output file from this request and open it
        std::string input_file_name = request->getOutputFilename();
        std::cout << "Adding contents of " << input_file_name << " to " << output_file_name << std::endl;
        std::ifstream input_file(input_file_name);
        output_file << input_file.rdbuf(); // Copy this chunk to the output file
        input_file.close();
    }
    output_file.close();
}

void cleanup(std::vector<HTTPGet*>& requests)
{
    for (HTTPGet* request: requests) {
        delete request;
    }
}

/*
 * Thread proc to run the IO service.  This fuction is ONLY used
 * when running in parallel mode and is not called directly.  It is
 * the thread proc used by the worker threads.
 */
void run_io_service(boost::asio::io_service *io_service)
{
    io_service->run();
}

/*
 * The user has specified that we will download the chunks simultaneously.
 * All the HTTPGet object has been created and now we just need to get the
 * io_service running on one or more threads.
 */
void downloadInParallel(int num_threads, boost::asio::io_service *io_service)
{
    // Validate num_threads and set a default
    if (num_threads <= 0) {
        num_threads == 1;
    }
    if (num_threads == 1) {
        // The user did not specify the thread count, so just run on the main thread
        io_service->run();
    } else {
        // User the number of threads the user specified to download the file.  All threads
        // use the one and only io_service.  Create all the threads and wait for them to finish.
        std::cout << "Creating " << num_threads << " threads to perform the download" << std::endl;
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; i++) {
            threads.push_back(std::thread(run_io_service, io_service));
        }
        for (std::thread &t: threads) {
            if (t.joinable()) {
                t.join();
            }
        }
    }
}

int getFileSize(const std::string& filename)
{
    struct stat statbuf;
    
    if (stat(filename.c_str(), &statbuf) == -1) {
        return 0;
    } else {
        return statbuf.st_size;
    }
}

