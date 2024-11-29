# Define color codes
RED    = \033[31m
GREEN  = \033[32m
YELLOW = \033[33m
BLUE   = \033[34m
RESET  = \033[0m

all: reader writer
	@echo "$(GREEN)Compile done, please start writer first, then start reader in another terminal$(RESET)"; 
	@echo "$(RED)Command: ./reader | ./writer (LINUX)$(RESET)"; 

reader:
	@echo "$(GREEN)Compiling reader.cpp ...$(RESET)"; 
	g++ -o reader reader.cpp;

writer:
	@echo "$(GREEN)Compiling writer.cpp ...$(RESET)"; 
	g++ -o writer writer.cpp;

clean:
	@echo "\n$(GREEN)Cleaning all C binaries..."; 
	@rm -rf reader;
	@rm -rf writer;
	@echo "Completed cleaning$(RESET)";