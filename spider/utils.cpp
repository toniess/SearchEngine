#include "utils.h"

#include <codecvt>
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include <regex>
#include "logger.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ip = boost::asio::ip;
namespace ssl = boost::asio::ssl;

using tcp = boost::asio::ip::tcp;

bool isText(const boost::beast::multi_buffer::const_buffers_type& b)
{
	for (auto itr = b.begin(); itr != b.end(); itr++)
	{
		for (int i = 0; i < (*itr).size(); i++)
		{
			if (*((const char*)(*itr).data() + i) == 0)
			{
				return false;
			}
		}
	}

	return true;
}

std::string getHtmlContent(const Link& link)
{

	std::string result;

	try
	{
		std::string host = link.hostName;
		std::string query = link.query;

		net::io_context ioc;

		if (link.protocol == ProtocolType::HTTPS)
		{

			ssl::context ctx(ssl::context::tlsv13_client);
			ctx.set_default_verify_paths();

			beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
			stream.set_verify_mode(ssl::verify_none);

			stream.set_verify_callback([](bool preverified, ssl::verify_context& ctx) {
				return true; // Accept any certificate
				});


			if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
				beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
				throw beast::system_error{ec};
			}

			ip::tcp::resolver resolver(ioc);
			get_lowest_layer(stream).connect(resolver.resolve({ host, "https" }));
			get_lowest_layer(stream).expires_after(std::chrono::seconds(30));


			http::request<http::empty_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			stream.handshake(ssl::stream_base::client);
			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			if (isText(res.body().data()))
			{
				result = buffers_to_string(res.body().data());
			}
			else
			{
				std::cout << "This is not a text link, bailing out..." << std::endl;
			}

			beast::error_code ec;
			stream.shutdown(ec);
			if (ec == net::error::eof) {
				ec = {};
			}

			if (ec) {
				throw beast::system_error{ec};
			}
		}
		else
		{
			tcp::resolver resolver(ioc);
			beast::tcp_stream stream(ioc);

			auto const results = resolver.resolve(host, "http");

			stream.connect(results);

			http::request<http::string_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);


			http::write(stream, req);

			beast::flat_buffer buffer;

			http::response<http::dynamic_body> res;


			http::read(stream, buffer, res);

			if (isText(res.body().data()))
			{
				result = buffers_to_string(res.body().data());
			}
			else
			{
				std::cout << "This is not a text link, bailing out..." << std::endl;
			}

			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);

			if (ec && ec != beast::errc::not_connected)
				throw beast::system_error{ec};

		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return result;
}


ProtocolType parse_protocol(const std::string& protocol_str) {
    if (protocol_str == "http") return ProtocolType::HTTP;
    return ProtocolType::HTTPS;
}

Link parse_link(const std::string& url) {
    std::regex url_regex(R"((https?)://([^/]+)(.*))", std::regex::icase);
    std::smatch match;

    // Check if the URL starts with http or https
    if (std::regex_match(url, match, url_regex)) {
        std::string protocol_str = match[1].str();
        std::string host_name = match[2].str();
        std::string query = match[3].str();

        ProtocolType protocol = parse_protocol(protocol_str);
        return { protocol, host_name, query };
    } else {
        // Handle URLs without a specified protocol
        std::regex no_protocol_regex(R"(([^/]+)(.*))");
        if (std::regex_match(url, match, no_protocol_regex)) {
            std::string host_name = match[1].str();
            std::string query = match[2].str();

            // Default to HTTPS if no protocol is specified
            return { ProtocolType::HTTPS, host_name, query };
        } else {
            throw std::invalid_argument("Invalid URL format");
        }
    }
}


std::vector<Link> extract_links(const std::string &html) {
    std::vector<Link> links;

    std::regex link_regex(R"(<a\s+(?:[^>]*?\s+)?href="([^"]*)\")", std::regex::icase);
    auto links_begin = std::sregex_iterator(html.begin(), html.end(), link_regex);
    auto links_end = std::sregex_iterator();

    for (std::sregex_iterator i = links_begin; i != links_end; ++i) {
        std::smatch match = *i;
        std::string url = match[1].str();
        try {
            links.push_back(parse_link(url));
        } catch (const std::invalid_argument&) {
            //Logger::instance().log("Ignoring invalid link: " + url);
        }
    }

    return links;
}

std::vector<Link> get_new_unique_links(const std::vector<Link> &links, std::set<Link> &existing_links) {
    std::vector<Link> new_links;

    for (const Link& link : links) {
        if (existing_links.find(link) == existing_links.end()) {
            new_links.push_back(link);
            existing_links.insert(link);
        }
    }

    return new_links;
}

std::string toLower(const std::string& input) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring winput = converter.from_bytes(input);
    std::wstring woutput;

    for (wchar_t ch : winput) {
        woutput += towlower(ch);
    }

    return converter.to_bytes(woutput);
}

std::unordered_map<std::string, int> countWordFrequency(std::string text) {
    Logger::instance().log("Indexing site...");
    std::unordered_map<std::string, int> wordCount;

    // delete tags
    std::regex tagPattern("<[^>]*>");
    text = std::regex_replace(text, tagPattern, " ");

    // replace with space if non letter or digit
    std::replace_if(text.begin(), text.end(), [](char c) {
            return !std::isalnum(static_cast<unsigned char>(c));
        }, ' ');

    std::istringstream stream(text);
    std::string word;

    while (stream >> word) {
        if (!word.empty()) {
            word = toLower(word);
            ++wordCount[word];
        }
    }
    Logger::instance().log("Indexing site success");
    return wordCount;
}
