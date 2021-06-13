all: run
 
HelloWorld.class: HelloWorld.java
	javac HelloWorld.java
 
hello_world: hello_world.c
	gcc -D__int64="long long" -o hello_world hello_world.c \
	   	-I/usr/lib/jvm/java-8-openjdk-amd64/include/linux/ \
		-I/usr/lib/jvm/java-8-openjdk-amd64/include/ \
	    -L/usr/lib/jvm/java-8-openjdk-amd64/jre/lib/amd64/server/ \
		-ljvm
  			
															   
run: HelloWorld.class hello_world
	./hello_world
 
clean:
	rm -f HelloWorld.class hello_world
