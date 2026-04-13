# MyProject

개인적으로 만드는 중인 게임 서버 프로젝트입니다.

## 개발 환경
- Visual Studio 2026
- vcpkg

## 빌드 방법

### 1. vcpkg 설치
```
git clone https://github.com/microsoft/vcpkg
cd vcpkg
bootstrap-vcpkg.bat
```


### 2. vcpkg 연동
```
vcpkg integrate install
```

### 3. 라이브러리 설치
```
cd [프로젝트 폴더]
vcpkg install --triplet x64-windows-static
```
### 4. 빌드
Visual Studio에서 .sln 파일 열고 빌드
