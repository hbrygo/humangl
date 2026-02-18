SRCS	=	src/main.cpp \
			src/glad.c \

OBJS	= ${SRCS:.cpp=.o}
OBJS	:= ${OBJS:.c=.o}
INCS	= Includes
GLAD_INC = glad
NAME	= humangl
CC      = cc
CXX     = c++
RM		= rm -rf
CFLAGS  = -Wall -Wextra -Werror -g -std=c11
CXXFLAGS= -Wall -Wextra -Werror -g -std=c++11
LDLIBS	= -lglfw -lGL -ldl

.cpp.o:
		${CXX} ${CXXFLAGS} -c $< -o ${<:.cpp=.o} -I ${INCS} -I ${GLAD_INC}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@ -I ${INCS} -I ${GLAD_INC}

${NAME}: ${OBJS}
	${CXX} ${OBJS} ${CXXFLAGS} ${LDLIBS} -o ${NAME}

all: ${NAME}

clean:
		${RM} ${OBJS}

fclean: clean
	${RM} ${NAME}

re: fclean all

.PHONY: all clean fclean re