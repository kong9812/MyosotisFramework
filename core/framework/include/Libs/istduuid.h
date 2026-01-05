// Copyright (c) 2025 kong9812
#pragma once
#include <uuid.h>

namespace uuids
{
	inline uuids::uuid hashMaker()
	{
		std::random_device rd;
		auto seed_data = std::array<int, 6>{};
		std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
		std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
		std::ranlux48_base generator(seq);
		uuids::basic_uuid_random_generator<std::ranlux48_base> gen(&generator);
		uuids::uuid const id = gen();
		return id;
	}
}