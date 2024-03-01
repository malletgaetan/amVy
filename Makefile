NAME = amVy
CC = clang
CFLAGS = -Wall -Werror -Wextra -std=gnu99 -I .
RM = rm -f

_SRCS=$(shell find . -name '*.c' ! -name "*.test.c" ! -name "main.c")
SRCS=$(_SRCS) main.c

_OBJS=$(_SRCS:.c=.o)
OBJS=$(SRCS:.c=.o)

ifeq ($(MAKECMDGOALS), debug)
  CFLAGS += -D DEBUG
endif

all: $(NAME)

$(NAME): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $(<:.c=.o)

debug: fclean $(NAME)

lexer_tests: $(_OBJS)
	$(CC) $(CFLAGS) -c lexer/lexer.test.c -o lexer/lexer.test.o
	$(CC) $(CFLAGS) -o lexer_tests $(_OBJS) lexer/lexer.test.o
	./lexer_tests
	$(RM) lexer_tests

tests: lexer_tests

clean:
	$(RM) -r **/*.o

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re tests lexer_tests debug
