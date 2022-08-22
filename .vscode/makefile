#source file search paths
VPATH = ./src 

	# compiler include search paths
INCS =  \
	-I./src/ravencode/pub/src \

LIBS = \
	-lws2_32 -lwsock32 \
	-lgdiplus -lgdi32 -lcomdlg32 -lstdc++fs \


static_link_flags = -static-libstdc++ -static-libgcc -static

# folders
ODIR = ./.vscode/obj
RDIR = ../ravencode/pub/src

OPT = -O2
CCflags = -std=c++17 $(OPT) -D_mingw_

# sources
OBJ_in2out = \
	$(ODIR)/in2out.o  \
	$(ODIR)/process.o  \
	$(ODIR)/ctcp.o  \

### COMPILE

$(ODIR)/ctcp.o: $(RDIR)/ctcp.cpp
	gcc	 -c -o $@ $<

$(ODIR)/%.o: %.cpp in2out.h
	g++  $(CCflags) \
	-c -o $@ $< $(INCS)

in2out: $(OBJ_in2out)
	g++ $(OPT) \
	-o bin/in2out.exe $^  \
	$(static_link_flags) $(LIBS)

.PHONY: clean
clean:
	del /q .vscode\\obj\\*.o

