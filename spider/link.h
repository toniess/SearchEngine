#pragma once 
#include <string>

enum class ProtocolType
{
	HTTP = 0,
	HTTPS = 1
};

struct Link
{
	ProtocolType protocol;
	std::string hostName;
	std::string query;

	bool operator==(const Link& l) const
	{
		return protocol == l.protocol
			&& hostName == l.hostName
			&& query == l.query;
	}

    bool operator<(const Link& l) const {
        if (static_cast<int>(protocol) < static_cast<int>(l.protocol)) return true;
        if (static_cast<int>(protocol) > static_cast<int>(l.protocol)) return false;

        if (hostName < l.hostName) return true;
        if (hostName > l.hostName) return false;

        return query < l.query;
    }

    std::string toString() const {
        return (protocol == ProtocolType::HTTP ? "http" : "https")
               + std::string("://")
               + hostName
               + query;
    }
};

