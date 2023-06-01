
all: run-ut run-hello-world

# ---------------------------- unit test ----------------------------
UNIT_TEST_SRCS=$(wildcard cpp_free_mock/unit_test/*.cpp)
UNIT_TEST_BINARY=$(patsubst %.cpp, build64_release/%, $(UNIT_TEST_SRCS))

build-ut:
	@bear blade build cpp_free_mock/unit_test

.PHONY: run-ut
run-ut: build-ut
	@echo $(UNIT_TEST_BINARY) | tr " " "\n" | while read test; do echo "\n\nTest: $$test"; ./$$test; done

# --------------------------- hello world ---------------------------
build-hello-world:
	@bear blade build cpp_free_mock/hello_world

.PHONY: run-hello-world
run-hello-world: build-hello-world
	@echo "\n\nTest: ./build64_release/cpp_free_mock/hello_world/hello_world"
	./build64_release/cpp_free_mock/hello_world/hello_world
