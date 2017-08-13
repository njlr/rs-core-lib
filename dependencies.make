$(BUILD)/common-test.o: rs-core/common-test.cpp rs-core/common.hpp rs-core/unit-test.hpp
$(BUILD)/container-test.o: rs-core/container-test.cpp rs-core/common.hpp rs-core/container.hpp rs-core/string.hpp rs-core/unit-test.hpp
$(BUILD)/float-test.o: rs-core/float-test.cpp rs-core/common.hpp rs-core/float.hpp rs-core/unit-test.hpp
$(BUILD)/io-test.o: rs-core/io-test.cpp rs-core/common.hpp rs-core/io.hpp rs-core/string.hpp rs-core/unit-test.hpp
$(BUILD)/kwargs-test.o: rs-core/kwargs-test.cpp rs-core/common.hpp rs-core/kwargs.hpp rs-core/unit-test.hpp
$(BUILD)/meta-test.o: rs-core/meta-test.cpp rs-core/common.hpp rs-core/meta.hpp rs-core/string.hpp rs-core/unit-test.hpp
$(BUILD)/random-test.o: rs-core/random-test.cpp rs-core/common.hpp rs-core/float.hpp rs-core/random.hpp rs-core/string.hpp rs-core/unit-test.hpp rs-core/vector.hpp
$(BUILD)/string-test.o: rs-core/string-test.cpp rs-core/common.hpp rs-core/string.hpp rs-core/unit-test.hpp
$(BUILD)/thread-test.o: rs-core/thread-test.cpp rs-core/common.hpp rs-core/thread.hpp rs-core/time.hpp rs-core/unit-test.hpp
$(BUILD)/time-test.o: rs-core/time-test.cpp rs-core/common.hpp rs-core/time.hpp rs-core/unit-test.hpp
$(BUILD)/uuid-test.o: rs-core/uuid-test.cpp rs-core/common.hpp rs-core/float.hpp rs-core/random.hpp rs-core/string.hpp rs-core/unit-test.hpp rs-core/uuid.hpp rs-core/vector.hpp
$(BUILD)/vector-test.o: rs-core/vector-test.cpp rs-core/common.hpp rs-core/float.hpp rs-core/string.hpp rs-core/unit-test.hpp rs-core/vector.hpp
