gcc -o game.x86_64 src/main.c -Iinclude -Llib/linux/x86_64 -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -DPLATFORM_DESKTOP
