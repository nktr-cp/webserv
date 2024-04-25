CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude -fsanitize=address,undefined
CXX = c++

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
MAIN =		main.cpp
# MAIN :=		$(addprefix main/, $(MAIN))

UTIL =		Utils.cpp \
			Errors.cpp
UTIL :=		$(addprefix Util/, $(UTIL))

CONFIG =	Config.cpp \
			Parse.cpp
CONFIG :=	$(addprefix Config/, $(CONFIG))

SERVER =	Server.cpp \
			Location.cpp
SERVER := $(addprefix Server/, $(SERVER))

CLIENT = Client.cpp
CLIENT := $(addprefix Client/, $(CLIENT))

LOCATION = Location.cpp
LOCATION := $(addprefix Location/, $(LOCATION))
##############################################################################

SRCS =	$(MAIN) $(UTIL) $(CONFIG) $(SERVER) $(CLIENT)
SRCS :=	$(addprefix $(SRCSDIR)/, $(SRCS))
OBJS =	$(SRCS:$(SRCSDIR)/%.cpp=$(OBJSDIR)/%.o)

all: $(NAME)

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

fclean: clean
	@$(RM) $(NAME)
	@printf "$(RED)Removed $(NAME)$(RESET)\n"

re: fclean all

bonus:

.PHONY: all clean fclean re bonus
