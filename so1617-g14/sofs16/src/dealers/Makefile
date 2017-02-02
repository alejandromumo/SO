CXX = g++
CXXFLAGS = -Wall
CXXFLAGS += -I "../rawdisk"
CXXFLAGS += -I "../probing"
CXXFLAGS += -I "../exception"
CXXFLAGS += -I "../core"
CXXFLAGS += -I "../dealers"
CXXFLAGS += -I "../freelists"
CXXFLAGS += -I "../filecluster"

LIB_NAME = sofs16Dealers

TARGET_LIB = lib$(LIB_NAME).a

# uncomment those that we want to include
OBJS =
OBJS += dealers.o
OBJS += sbdealer.o
OBJS += czdealer.o
#OBJS += itdealer.o

all:			$(TARGET_LIB)

$(TARGET_LIB):		$(OBJS)
	ar -r $(TARGET_LIB) $^
	cp $(TARGET_LIB) ../../lib
	rm -f $^ $(TARGET_LIB)

clean:
	rm -f $(OBJS) $(TARGET_LIB)
	rm -f *~

cleanall:	clean
	rm -f ../../lib/$(TARGET_LIB)
