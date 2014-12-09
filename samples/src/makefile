LIBS = -lGL -lGLU -lglut -lassimp -lIL -lfreetype -lGLEW -L/usr/lib/x86_64-linux-gnu


samples : ${SOURCE}.cpp
	g++ -o $@ $< ${LIBS} -I/usr/include/freetype2

clean:
	rm -rf *.o ${PROG}
