#pragma once

//무슨 용도냐?
/*
왜 만드냐? NetAddress가 왜 있어야되냐?
1. 서버의 리슨소켓을 관리하기 위해서
2. 클라이언트의 소켓을 관리하기 위해서
3. DB의 IP주소와 포트번호를 관리하기 위해서
그럼 뭐가 필요하냐?
1. IP주소와 포트번호를 저장할 수 있는 멤버변수
2. IP주소와 포트번호를 설정할 수 있는 멤버함수
3. IP주소와 포트번호를 반환할 수 있는 멤버함수
4. IP주소와 포트번호를 문자열로 반환할 수 있는 멤버함수
*/

class NetAddress
{
public:
	NetAddress();
	~NetAddress();

	int16 _port;
private:
	SOCKADDR_IN _sockAddr;
};

