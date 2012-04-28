clang -o meme *.c \
    `pkg-config --cflags --libs glib-2.0` \
    -std=gnu1x -fplan9-extensions \
    -Wall -Wextra -Werror=implicit-function-declaration -Wno-unused-parameter \
    -Wno-incompatible-pointer-types \
    -g
