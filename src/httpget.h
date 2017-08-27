
#ifndef __multiget_http_get_include__
#define __multiget_http_get_include__

#include <string>
#include <fstream>
#include <boost/asio.hpp>
using namespace boost::asio::ip;

/*! \brief Get a file from the internet (the entire file or a range of bytes)
 *
 *  HTTPGet uses asynchronous IO to pull a file from a URL.  It will use a start
 *  and end range to determine which bytes to download.  If start and end are both
 *  set to 0 it will pull the entire file in a single request.
 */
class HTTPGet {
public:
	/**
     *   @brief  Create a HTTPGet object.
     *
     *   @param  io_service
     *   @param  server dns name of server or IP address
	 *   @param  path path to file e.g. /pathtofile.extention
	 *   @param  port either \"http\" or port number
	 *   @param  start_range Used in HTTP Range header to indicate beginning byte
	 *   @param  end_range Used in HTTP Range header to indicate end byte
	 *   @param  output_file_name Where to store the body of the HTTP request
	 *
     *   @return HTTPGet object
    */
	HTTPGet(boost::asio::io_service& io_service, const std::string& server, const std::string& path, const std::string& port, int start_range, int end_range,
			const char* output_file_name);
	virtual ~HTTPGet();

	/**
	 *   @brief  Get the name of the file where the HTTP body was stores.
	 *
	 *   This file will get deleted when this HTTPGet object is destroyed.
	 *
	 *   @return filename The name of the file that contains the output from the HTTP GET
	 */
	std::string getOutputFilename() { return output_file_name_; }

private:
	/**
	 *   @brief  boost asio tcp async function
	 *
	 *   See the boost asio documentation for more details
	 *
	 */
	void handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator);
	void handle_connect(const boost::system::error_code& err);
	void handle_write_request(const boost::system::error_code& err);
	void handle_read_status_line(const boost::system::error_code& err);
	void handle_read_headers(const boost::system::error_code& err, size_t bytes);
	void handle_read_content(const boost::system::error_code& err);

	int								start_range_; // first byte to get in Range
	int								end_range_; // last byte to get in Range
	// The following are needed for the boost::asio functions
	boost::asio::ip::tcp::resolver	resolver_;
	boost::asio::ip::tcp::socket	socket_;
	boost::asio::streambuf			request_;
	boost::asio::streambuf			response_;

	std::string						output_file_name_; // Keep track of what file we used
	std::ofstream					output_file_; // A stream to the above file

	// Ensure that these method are not created explicitly
	HTTPGet(); // not implemented
	HTTPGet(const HTTPGet& in); // not implemented
	HTTPGet& operator = (const HTTPGet &t); // not implemented
};


#endif // __multiget_http_get_include__
