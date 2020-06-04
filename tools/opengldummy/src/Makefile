
all: libEGL.so libGLESv2.so

libEGL.so: egl.cpp
	${CC} -DQGS_BUILD_CLIENT_DLL -fPIC -shared -I. -I../3rdparty/include -o libEGL.so egl.cpp
libGLESv2.so: gles2.cpp
	${CC} -DQGS_BUILD_CLIENT_DLL -fPIC -shared -I. -I../3rdparty/include -o libGLESv2.so gles2.cpp

