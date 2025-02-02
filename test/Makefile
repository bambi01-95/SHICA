# ソースディレクトリ
SRC_DIR := ../test

# 拡張子 .txt のファイルを取得
SRC_FILES := $(wildcard $(SRC_DIR)/*.txt)

# 拡張子 .txt のファイルを取得
SRC_OUT_FILES := $(wildcard $(SRC_DIR)/*.stt)

# ターゲットを定義
com: process_all_com
run: process_all_run

# 各 .txt ファイルに対して ./shica を実行
process_all_com: $(SRC_FILES)
	@for file in $^; do \
		echo "Processing $$file"; \
		../source/shica "$$file"; \
	done

# 各 .txt ファイルに対して ./shica を実行
process_all_run: $(SRC_OUT_FILES)
	@for file in $^; do \
		echo "Processing $$file"; \
		../source/vm "$$file"; \
	done

# クリーンターゲット（特に不要だが、念のため）
clean: $(SRC_OUT_FILES)
	@for file in $^; do \
		rm -f "$$file"; \
	done
	

.PHONY: all process_all_com clean