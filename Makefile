# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hucorrei <hucorrei@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/07/31 13:35:34 by hucorrei          #+#    #+#              #
#    Updated: 2024/01/16 11:12:37 by hucorrei         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

################################################################################
#                                     CONFIG                                   #
################################################################################

NAME		= webserv
CC			= c++
CFLAGS		= -Wall -Wextra -Werror -std=c++98
OBJ_DIR		= .objs
RM			= rm -rf

################################################################################
#                                 PROGRAM'S SRCS                               #
################################################################################

SRCS		= $(wildcard $(SRC_DIR)*.cpp)

OBJS		= $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

################################################################################
#                                  Makefile  objs                              #
################################################################################

CLR_RMV		= \033[0m
RED		    = \033[1;31m
GREEN		= \033[1;32m
YELLOW		= \033[1;33m
CYAN 		= \033[1;36m
RM			= rm -rf

all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(GREEN)Compilation ${CLR_RMV}of ${YELLOW}$(NAME) ${CLR_RMV}..."
	@$(CC) $(OBJS) $(CFLAGS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created[0m ✔️"

clean:
	@$(RM) $(OBJ_DIR)
	@ echo "$(RED)Deleting $(CYAN)$(NAME) $(CLR_RMV)objs ✔️"

fclean: clean
	@$(RM) $(NAME)
	@ echo "$(RED)Deleting $(CYAN)$(NAME) $(CLR_RMV)binary ✔️"

re: fclean all

run: all
	./$(NAME)

git:
		git add .
		git commit -m "$m"
		git push
		@ echo "$(BLUE)ALL is on your $(CYAN)GIT $(CLR_RMV)✔️"

.PHONY:	all clean fclean re run git