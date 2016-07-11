build/$(TARGET)/core-test.o: prion/core-test.cpp prion/core.hpp prion/unit-test.hpp
LDLIBS += -lpthread -lz
