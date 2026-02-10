NAME = ft_traceroute
CC = cc
# CFLAGS = -Wall -Wextra -Werror
SRCS = ft_traceroute.c argparse.c error.c
OBJS = $(SRCS:.c=.o)

INCLUDES = -I.

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(INCLUDES) -o $@ $^ -lm

%.o: %.c
	$(CC) $(INCLUDES) -c $< -o $@ 

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)
	
re: fclean all