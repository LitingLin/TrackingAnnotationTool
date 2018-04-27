#pragma once
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/binary.hpp>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Rpc.h>

template<class Archive>
void save(Archive & archive,
	GUID const & m)
{
	std::string str;
	str.resize(sizeof(GUID));
	memcpy(&str[0], &m, sizeof(GUID));
	archive(str);
}

template<class Archive>
void load(Archive & archive,
	GUID & m)
{
	std::string str;
	archive(str);
	if (str.size() < sizeof(GUID))
		throw cereal::Exception("Failed in parsing GUID");

	memcpy(&m, &str[0], sizeof(GUID));
}