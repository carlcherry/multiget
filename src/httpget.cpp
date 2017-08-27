#include "httpget.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <iostream>

HTTPGet::HTTPGet(boost::asio::io_service& io_service, const std::string& server, const std::string& path, const std::string& port, int start_range,
				 int end_range, const char* output_file_name)
: start_range_(start_range)
, end_range_(end_range)
, resolver_(io_service)
, socket_(io_service)
, output_file_name_(output_file_name)
, output_file_(output_file_name, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc)
{
	// Create the HTTP request to get part of the file
	std::ostream request_stream(&request_);
	request_stream << "GET " << path << " HTTP/1.1\r\n";
	request_stream << "Host: " << server << "\r\n";
	// If end_range_ is set then we include the header, if it is 0 then the caller
	// must want the entire file
	if (end_range_ != 0) {
		request_stream << "Range: " << "bytes=" << start_range << "-" << end_range << "\r\n";
	}
	// Once the file is downloaded - close the connection
	request_stream << "Connection: close\r\n\r\n";
	
	// We are only supporting the http protocol for this implementation (no HTTPS).  However,
	// the use can specifiy a port other than 80 using the port paramater
	tcp::resolver::query query(server, port);
	// Resolve the server address (async)
	resolver_.async_resolve(query,
							boost::bind(&HTTPGet::handle_resolve, this,
										boost::asio::placeholders::error,
										boost::asio::placeholders::iterator));
}

HTTPGet::~HTTPGet()
{
	unlink(output_file_name_.c_str());
}

void HTTPGet::handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// Attempt a connection to each endpoint in the list until we
		// successfully establish a connection.
		boost::asio::async_connect(socket_, endpoint_iterator,
								   boost::bind(&HTTPGet::handle_connect, this, boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void HTTPGet::handle_connect(const boost::system::error_code& err)
{
	if (!err)
	{
		// We are connected - send the HTTP request to get a chunk of the file
		boost::asio::async_write(socket_, request_,
								 boost::bind(&HTTPGet::handle_write_request, this, boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void HTTPGet::handle_write_request(const boost::system::error_code& err)
{
	if (!err)
	{
		// Read the HTTP responce (i.e. up until the first \r\n
		boost::asio::async_read_until(socket_, response_, "\r\n",
									  boost::bind(&HTTPGet::handle_read_status_line, this, boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err.message() << "\n";
	}
}

void HTTPGet::handle_read_status_line(const boost::system::error_code& err)
{
	if (!err)
	{
		// Check that response is OK.
		std::istream response_stream(&response_);
		std::string http_version;
		response_stream >> http_version;
		unsigned int status_code;
		response_stream >> status_code;
		std::string status_message;
		std::getline(response_stream, status_message);
		if (!response_stream || http_version.substr(0, 5) != "HTTP/")
		{
			std::cout << "Invalid response\n";
			return;
		}
		if (status_code != 200 && status_code != 206)
		{
			std::cout << "Response returned with status code ";
			std::cout << status_code << "\n";
			return;
		}
		
		// Read the response headers, which are terminated by a blank line.
		boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
									  boost::bind(&HTTPGet::handle_read_headers, this,
												  boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		std::cout << "Error: " << err << "\n";
	}
}

void HTTPGet::handle_read_headers(const boost::system::error_code& err, size_t bytes)
{
	if (!err)
	{
		// Process the response headers.
		std::istream response_stream(&response_);
		std::string header;
		while (std::getline(response_stream, header) && header != "\r") {
			//std::cout << header << std::endl;
		}

		// We have finished reading all the headers...
		// Now check to see if we have any of the body in the stream.
		size_t avail = response_stream.rdbuf()->in_avail();
		if (avail > 0) {
			// Read in the remaining bytes
			handle_read_content(err);
		}

		// Continue reading asynchronously until EOF
		boost::asio::async_read(socket_, response_,
								boost::asio::transfer_at_least(1),
								boost::bind(&HTTPGet::handle_read_content, this,
											boost::asio::placeholders::error));
	}
	else
	{
		std::cout << "Error: " << err << "\n";
	}
}

void HTTPGet::handle_read_content(const boost::system::error_code& err)
{
	if (!err) {
		// Write all of the data that has been read so far.
		//std::cout << start_range_ << " got some content" << std::endl;
		output_file_ << &response_;
		
		// Continue reading remaining data until EOF.
		boost::asio::async_read(socket_, response_,
								boost::asio::transfer_at_least(1),
								boost::bind(&HTTPGet::handle_read_content, this,
											boost::asio::placeholders::error));
	} else if (err != boost::asio::error::eof) {
		std::cout << "Error: " << err << "\n";
	} else {
		// We are at the end of the file - close the output file
		output_file_.flush();
		output_file_.close();
	}
}

