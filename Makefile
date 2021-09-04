CPPGNU = gcc

SOURCE = src/

TARGET = output

all: $(TARGET)

rebuild: all

$(TARGET) :
	$(CPPGNU) $(SOURCE)$(TARGET).c -o $(TARGET)

clean :
	-rm -f $(TARGET)
