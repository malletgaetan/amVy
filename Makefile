NAME = amVy
CC = clang
CFLAGS = -Wall -Werror -Wextra
RM = rm -f

SRCS=$(shell find . -name '*.c')
OBJS=$(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
