CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude -fsanitize=address,undefined
NAME = webserv

GRAY		= \033[1;37m
CYAN		= \033[1;36m
MAGENTA		= \033[1;35m
BLUE		= \033[1;34m
YELLOW		= \033[1;33m
GREEN		= \033[1;32m
RED			= \033[1;31m
RESET		= \033[0m
MAX			= 50
CONVERSION	= %-$(MAX).$(MAX)s\r

SRCSDIR = src
OBJSDIR = obj

##############################################################################
SRCS =	main.cpp \
		webserv.cpp \
		server.cpp \
		server_config.cpp \
		location.cpp \
		config.cpp \
		errors.cpp \
		utils.cpp \
		http_request.cpp \
		http_response.cpp \
		request_handler.cpp \
		session.cpp \
		cgi.cpp \
##############################################################################

SRCS := $(addprefix $(SRCSDIR)/, $(SRCS))
OBJS = $(SRCS:$(SRCSDIR)/%.cpp=$(OBJSDIR)/%.o)

all: $(NAME)
.PHONY: all

linux:
	docker build -t webserv .
	docker run -p 8080:8080 --name webserv_container webserv || true
	docker rm webserv_container
.PHONY: linux

$(NAME): $(OBJS)
	@printf "$(YELLOW)Compiling $@... $(CONVERSION)$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@printf "$(GREEN)⪼ $(NAME): compilation done ⪻$(CONVERSION)$(RESET)\n"

$(OBJSDIR)/%.o: $(SRCSDIR)/%.cpp
	@mkdir -p $(OBJSDIR) $(dir $@)
	@printf "$(MAGENTA)Compiling $@... $(CONVERSION)$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) -r $(OBJSDIR)
	@printf "$(RED)Removed $(NAME)'s object files$(RESET)\n"
.PHONY: clean

fclean: clean
	@$(RM) $(NAME)
	@printf "$(RED)Removed $(NAME)$(RESET)\n"
.PHONY: fclean

re: fclean all
.PHONY: re
