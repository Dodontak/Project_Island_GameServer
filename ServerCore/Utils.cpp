#include "Utils.h"
#include <jwt-cpp/jwt.h>
#include <random>

using namespace std;

mutex Utils::m;

int Utils::ErrorExit(const char* errstr)
{
	cerr << errstr << endl;
	exit(1);
	return 1;
}

int32 Utils::GetRandNum(int32 start, int32 end)
{
	::mt19937 rng(random_device{}());
	::uniform_int_distribution<int32> dist(start, end);
	return dist(rng);
}

bool Utils::VerifyAccessToken(const string& token, string& out_user_id, string& out_nickname)
{
	// TODO 비밀키 환경변수에 저장하거나 다른방법으로 가져와야함.
	const string SECRET_KEY = "cb1c63a81ccd9488c37de67a6028996ca0d994f1f22d05a84818a8a770e028ab";

	try
	{
		auto verifier = jwt::verify()
			.allow_algorithm(jwt::algorithm::hs256{ SECRET_KEY })
			.with_issuer("auth_server")    // 발급자 확인
			.leeway(30);
		auto decoded = jwt::decode(token);
		verifier.verify(decoded);           // 서명 + 만료시간 자동 검증

		out_user_id = decoded.get_payload_claim("user_id").as_string();
		out_nickname = decoded.get_payload_claim("nickname").as_string();
		return true;
	}
	catch (const exception& e)
	{
		cout << "exeption " << e.what() << endl;
		// 서명 불일치, 만료, 형식 오류 전부 여기로 떨어짐
		return false;
	}
}

uint64 Utils::GetObjectId()
{
	static atomic<uint64> atomic_id = 0;
	uint64 id = atomic_id.fetch_add(1);
	return id;
}
