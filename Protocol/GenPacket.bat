pushd %~dp0

protoc -I=./ --cpp_out=./ ./Enum.proto
protoc -I=./ --cpp_out=./ ./Struct.proto
protoc -I=./ --cpp_out=./ ./Protocol.proto

GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=GC_ --send=GS_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv GS_ AS_ --send GC_ AC_

IF ERRORLEVEL 1 PAUSE

XCOPY /Y *.pb.h "..\GameServer\"
XCOPY /Y *.pb.cc "..\GameServer\"
XCOPY /Y ClientPacketHandler.h "..\GameServer\"

XCOPY /Y *.pb.h "..\DummyClient\"
XCOPY /Y *.pb.cc "..\DummyClient\"
XCOPY /Y ServerPacketHandler.h "..\DummyClient\"

IF ERRORLEVEL 1 PAUSE

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc

protoc_v21_12 -I=./ --cpp_out=./ ./Enum.proto
protoc_v21_12 -I=./ --cpp_out=./ ./Struct.proto
protoc_v21_12 -I=./ --cpp_out=./ ./Protocol.proto

XCOPY /Y *.pb.h "..\..\Client\Source\Client\Network\"
XCOPY /Y *.pb.cc "..\..\Client\Source\Client\Network\"
XCOPY /Y ServerPacketHandler.h "..\..\Client\Source\Client\"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

IF ERRORLEVEL 1 PAUSE

PAUSE