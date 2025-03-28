# ソースディレクトリ
SRC_DIR := ../test

# コンパイラとVMのディレクトリ
CODE_DIR := ../source

# 拡張子 .txt のファイルを取得
SRC_FILES := $(wildcard $(SRC_DIR)/*.txt)

# 拡張子 .txt のファイルを取得
SRC_OUT_FILES := $(wildcard $(SRC_DIR)/*.stt)

all: 
	gcc -o $(CODE_DIR)/shica $(CODE_DIR)/compiler.c -DTEST=0 -DDEBUG=0
	gcc -o $(CODE_DIR)/vm    $(CODE_DIR)/executor.c -DTEST=0 -DDEBUG=0
	make process_all_com
	make clean

# 各 .txt ファイルに対して ./shica を実行
process_all_com: $(SRC_FILES)
	@for file in $^; do \
		echo "Processing $$file"; \
		../source/shica "$$file"; \
	done

# 各 .stt ファイルに対して ./vm を実行
process_all_run: $(SRC_OUT_FILES)
	@for file in $^; do \
		echo "Processing $$file"; \
		../source/vm "$$file"; \
	done


# クリーンターゲット（特に不要だが、念のため）
clean: $(SRC_OUT_FILES)
	@for file in $(wildcard $(SRC_DIR)/*.stt); do \
		rm -f "$$file"; \
	done
	rm -f ../source/shica
	rm -f ../source/vm
	

.PHONY: 
	all 