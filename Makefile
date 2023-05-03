.PHONY: xx

"":
	if [ -d "build" ]; then \
		cd build && make -j$(nproc); \
	else \
		mkdir build; \
		cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_CXX_COMPILER:FILEPATH=$(shell which g++) -DCMAKE_C_COMPILER:FILEPATH=$(shell which gcc) ..; \
	fi

%:
	if [ -d "build" ]; then \
		cd build && make $@; \
	else \
		mkdir build; \
		cd build && cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_CXX_COMPILER:FILEPATH=$(shell which g++) -DCMAKE_C_COMPILER:FILEPATH=$(shell which gcc) $@ ..; \
	fi
