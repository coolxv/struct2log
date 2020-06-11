TARGET = test
SOURCES = test_log.cpp
WARNINGFLAGS = -Wall -W
CPPFLAGS = -g -std=c++11 -DSTRUCT2LOG_DEBUG
LINKS = -lpthread -ldl
CC = g++
OBJS = $(patsubst %.cpp,%.o,$(SOURCES))

%.o:%.cpp
	$(CC) -c $< -o $@ $(WARNINGFLAGS) $(CPPFLAGS)
    
$(TARGET):$(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LINKS)
    
clean:
	rm -rf $(OBJS) $(TARGET) *.log