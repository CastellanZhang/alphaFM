GCCVERSIONLT8 = $(shell expr `$(CXX) -dumpversion | cut -f1 -d.` \< 8)
CFLAGS = -O3 -w

ifeq "$(GCCVERSIONLT8)" "1"
	CFLAGS += -std=c++11
endif

all:
	$(CXX) $(CFLAGS) fm_train.cpp src/Frame/pc_frame.cpp src/Utils/utils.cpp -I . -o bin/fm_train -lpthread
	$(CXX) $(CFLAGS) fm_predict.cpp src/Frame/pc_frame.cpp src/Utils/utils.cpp -I . -o bin/fm_predict -lpthread
	$(CXX) $(CFLAGS) model_bin_tool.cpp src/Utils/utils.cpp -I . -o bin/model_bin_tool
