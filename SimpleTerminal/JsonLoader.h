#pragma once
#include <fstream>
#include <optional>
#include <filesystem>
#include <nlohmann\json.hpp>

template <typename T>
bool LoadJson(const std::filesystem::path& fileAddr, T& data)
{
	if (!std::filesystem::exists(fileAddr))
	{
		return false;
	}

	std::ifstream file(fileAddr);
	if (!file.is_open())
	{
		return false;
	}
	try
	{
		nlohmann::json j = nlohmann::json::parse(file);
		file.close();
		data = j.get<T>();
		return true;
	}
	catch (std::exception e)
	{
		file.close();
		return false;
	}
}

template <typename T>
bool SaveJson(const std::filesystem::path& fileAddr, const T& data)
{
	nlohmann::json j = data;
	std::ofstream file(fileAddr);
	if (!file.is_open())
	{
		return false;
	}
	try
	{
		file << j.dump(2);
		file.close();
		return true;
	}
	catch (std::exception e)
	{
		file.close();
		return false;
	}
}

template <typename T>
bool LoadBson(const std::filesystem::path& fileAddr, T& data)
{
	if (!std::filesystem::exists(fileAddr))
	{
		return false;
	}

	std::ifstream file(fileAddr, std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}
	std::vector<uint8_t> bson(std::istreambuf_iterator<uint8_t>(file), std::istreambuf_iterator<uint8_t>());
	try
	{
		nlohmann::json j = nlohmann::json::from_bson(bson);
		file.close();
		data = j.get<T>();
		return true;
	}
	catch (std::exception e)
	{
		file.close();
		return false;
	}
}

template <typename T>
bool SaveBson(const std::filesystem::path& fileAddr, const T& data)
{
	nlohmann::json j = data;
	std::ofstream file(fileAddr, std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}
	try
	{
		std::vector<uint8_t> bson = nlohmann::json::to_bson(j);
		file.write(bson.data(), bson.size());
		file.close();
		return true;
	}
	catch (std::exception e)
	{
		file.close();
		return false;
	}
}