emcc -o web-export/index.html src/main.c -Os -Wall ./lib/web/libraylib.a -I. -Iinclude -L. -Llib/web -s USE_GLFW=3 --shell-file minshell.html -DPLATFORM_WEB --preload-file assets
