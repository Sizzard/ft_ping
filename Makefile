NAME = ft_ping
SRCS = main.c srcs/opt.c srcs/ft_ping.c srcs/debug.c srcs/utils.c srcs/signal.c
OBJ = $(SRCS:.c=.o)
CFLAGS = -Wall -Wextra -Werror -g3

all: $(NAME)

$(NAME): $(OBJ)
	cc $(CFLAGS) -o $(NAME) $(OBJ)

%.o:%.c ft_ping.h
	cc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
