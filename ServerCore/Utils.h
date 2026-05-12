#pragma once

#include <mutex>
#include <random>
#include <string>
#include <iostream>
#include "Types.h"

class Utils
{
public:
	static int ErrorExit(const char* errstr);
	static int32 GetRandNum(int32 start, int32 end);
	static  bool VerifyAccessToken(const std::string& token, std::string& out_user_id, std::string& out_nickname);

	template<typename... Args>
	static void LockPrint(Args&&... args)
	{
		std::lock_guard<std::mutex> lock(m);
		(std::cout << ... << args) << std::endl;
	}

	template<typename T>
	static T GetRandom(T min, T max)
	{
		//시드값을 얻기 위한 random_device 생성
		std::random_device randomDevice;
		// random_device 를 통해 난수 생성 엔진을 초기화 한다
		std::mt19937 generator(randomDevice());
		// 균등하게 낭타나는 난수열을 생성하기 위해 균등 분포 정의;

		if constexpr (std::is_integral_v<T>)
		{
			std::uniform_int_distribution<T> distribution(min, max);
			return distribution(generator);
		}
		else
		{
			std::uniform_real_distribution<T> distribution(min, max);
			return distribution(generator);
		}
	}

private:
	static std::mutex m;
};

