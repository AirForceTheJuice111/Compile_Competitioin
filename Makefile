.PHONY: format

# 一键格式化所有 HW* 目录下的 .cc 和 .hh 文件
format:
	@echo "Formatting all .cc and .hh files under HW*/ ..."
	@find HW* -type f \( -name "*.cc" -o -name "*.hh" \) \
		! -path "*/build/*" ! -path "*/vendor/*" \
		| xargs clang-format -i
	@echo "Done."
