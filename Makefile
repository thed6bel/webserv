# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hucorrei <hucorrei@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/31 13:35:34 by hucorrei          #+#    #+#              #
#    Updated: 2024/04/17 11:13:27 by hucorrei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

################################################################################
#                                     CONFIG                                   #
################################################################################

NAME		= webserv
CC			= g++
CFLAGS		= -Wall -Wextra -Werror -std=c++98
OBJ_DIR		= .objs
SRC_DIR		= src/
RM			= rm -rf

################################################################################
#                                 PROGRAM'S SRCS                               #
################################################################################

SRCS 		= main.cpp \
			$(shell find $(SRC_DIR) -name '*.cpp')
OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

################################################################################
#                                COMPILATION RULES                             #
################################################################################

CLR_RMV		= \033[0m
RED			= \033[1;31m
GREEN		= \033[1;32m
YELLOW		= \033[1;33m
CYAN 		= \033[1;36m

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

################################################################################
#                                  Makefile  objs                              #
################################################################################

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)Linking ${CLR_RMV}$(YELLOW)$(NAME) $(CLR_RMV)..."
	@$(CC) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created $(CLR_RMV)✔️"

clean:
	@$(RM) $(OBJ_DIR)
	@echo "$(RED)Deleting $(CYAN)$(NAME) $(CLR_RMV)objs ✔️"

fclean: clean
	@$(RM) $(NAME)
	@echo "$(RED)Deleting $(CYAN)$(NAME) $(CLR_RMV)binary ✔️"

re: fclean all

run: all
	./$(NAME)

git:
		git add .
		git commit -m "$m"
		git push
		@echo "$(CYAN)ALL is on your GIT $(CLR_RMV)✔️"

.PHONY:	all clean fclean re run git
