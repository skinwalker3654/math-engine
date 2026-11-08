source = source/main.c source/tokenizer.c source/parser.c \
		 source/interpreter.c source/semantic.c source/ast.c

cc = gcc
target = exe

$(target): $(source)
	@echo "compiling math engine ..."
	$(cc) $^ -o $@ -lm

clean:
	@echo "removing math engine ..."
	rm $(target)

run:
	@echo "launching math engine ..."
	./$(target)

.PHONY: clean run
