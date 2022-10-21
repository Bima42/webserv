/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: trossel <trossel@42lausanne.ch>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/09/26 16:47:18 by trossel           #+#    #+#             */
/*   Updated: 2022/10/21 15:49:02 by trossel          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Config.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

ConfigParsor::ConfigParsor(const std::string &filename)
	: _filename(filename)
{
}

ConfigParsor::~ConfigParsor()
{
}

ConfigParsor::ConfigParsor(const ConfigParsor &rhs)
{
	*this = rhs;
}

ConfigParsor &ConfigParsor::operator=(const ConfigParsor &rhs)
{
	(void)rhs;
	return (*this);
}

const std::string &ConfigParsor::getFilename() const { return this->_filename;}

Location ConfigParsor::parseLocation(const JsonObject &locObject, const Location &defaultLoc) const
{
	std::map<std::string, RequestType> requests_map;
	requests_map["GET"] = Get;
	requests_map["POST"] = Post;
	requests_map["DELETE"] = Delete;
	requests_map["PUT"] = Put;
	requests_map["HEAD"] = Head;
	requests_map["PATCH"] = Patch;

	Location loc;

	loc.root_dir = locObject.getStringOrDefault("root", "");

	loc.auto_index = locObject.getBoolOrDefault("auto_index", defaultLoc.auto_index);
	loc.directory_listing = locObject.getBoolOrDefault("directory_listing", defaultLoc.directory_listing);

	loc.max_client_body_size = locObject.getIntOrDefault("max_client_body_size", defaultLoc.max_client_body_size);

	// CGI_extensions
	std::vector<std::string> cgi_ext = locObject.getArrayOrEmpty("cgi_extensions").stringValues();
	if (cgi_ext.empty())
		cgi_ext = defaultLoc.cgi_extensions;
	for(std::vector<std::string>::iterator it = cgi_ext.begin();
		it != cgi_ext.end(); it++)
	{
		std::string ext = *it;
		ft::trim(ext);
		loc.cgi_extensions.push_back(ext);
	}

	// Indexes
	std::vector<std::string> indexes = locObject.getArrayOrEmpty("index").stringValues();
	if (indexes.empty())
		indexes = defaultLoc.indexes;
	for (std::vector<std::string>::iterator it = indexes.begin();
		it != indexes.end(); it++)
	{
		std::string index = *it;
		ft::trim(index);
		loc.indexes.push_back(index);
	}

	std::vector<std::string> disabled_requests = locObject.getArrayOrEmpty("disabled_methods").stringValues();
	for (size_t i(0); i < disabled_requests.size(); i++)
	{
		RequestType type = requests_map[disabled_requests[i]];
		loc.disableRequest(type);
	}

	loc.cgi_bin = locObject.getStringOrDefault("cgi_bin", "");
	if (!loc.cgi_bin.empty())
		loc.isCGI = true;
	else
		loc.cgi_bin = defaultLoc.cgi_bin;

	return loc;
}

Server ConfigParsor::parseServer(const JsonObject &serverObject) const
{
    Server serverCfg;

    serverCfg.addPort(serverObject.getInt("port"));

	serverCfg.addAddress(serverObject.getString("address"));


	std::vector<std::string> hosts = serverObject.getArray("server_name").stringValues();
	for(std::vector<std::string>::iterator it = hosts.begin();
		it != hosts.end(); it++)
		serverCfg.addName(*it);

	// Default location
	Location defaultLocation = parseLocation(serverObject, Location());
	defaultLocation.isCGI = false;
	serverCfg.addLocation("", defaultLocation);

	// Other locations
	std::vector<JsonObject> locations = serverObject.getArray("locations").ObjectValues();
	for(std::vector<JsonObject>::iterator it = locations.begin();
		it != locations.end(); it++)
	{
		std::string location_path = it->getString("location_path");
		ft::trim(location_path);
		if (location_path.empty())
			throw std::logic_error("Location error: location_path cannot be empty");
		serverCfg.addLocation(location_path, parseLocation(*it, defaultLocation));
	}
	return serverCfg;
}

Config ConfigParsor::parse() const
{
	Config cfg;
	JsonObject json;

	cfg.setValid(false);
	try
	{
		json.parseFromFile(this->_filename);

		JsonArray serversArray = json.getArray("servers");
		std::vector<JsonObject> servers = serversArray.ObjectValues();

		std::vector<JsonObject>::const_iterator it;
		for(it = servers.begin(); it != servers.end(); it++)
			cfg.addServer(parseServer(*it));

		cfg.setValid(true);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	std::cout << "Config = " << cfg << std::endl;
	return cfg;
}
