#include "raylib.h"
#include "raymath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

const int screenWidth = 800;
const int screenHeight = 800;
const int PLAYER_SPEED = 10;
const int BOMBA_SPEED = 6;
Camera3D camera;
RenderTexture2D screen_tex;

RenderTexture2D pipa_tex;

Model pipa_model;

Texture bullet_texture;

Vector2 last_mouse_pos;

int game_state = 0;

struct Entity {
  struct Entity *next_entity;
  float x;
  float y;
  float z;
  float angle;
  Model model;
  bool dead;

  void (*Update)(struct Entity *player);
};

struct Entity *first_entity;

void UpdatePipa(struct Entity *pipa) {
  float angle = 0.0f;
  if (IsKeyDown(KEY_D)) {
    (*pipa).z -= PLAYER_SPEED * GetFrameTime();
    angle = 35.0f;
  } else if (IsKeyDown(KEY_A)) {
    (*pipa).z += PLAYER_SPEED * GetFrameTime();
    angle = -35.0f;
  }

  if (pipa->z > 8.5f)
    (*pipa).z = 8.5f;
  else if (pipa->z < -8.5f)
    (*pipa).z = -8.5f;

  if (IsKeyDown(KEY_W)) {
    (*pipa).y += PLAYER_SPEED * GetFrameTime();
  } else if (IsKeyDown(KEY_S)) {
    (*pipa).y -= PLAYER_SPEED * GetFrameTime();
  }

  if (pipa->y > 13.0f)
    (*pipa).y = 13.0f;
  else if (pipa->y < 1.0f)
    (*pipa).y = 1.0f;

  DrawModelEx(pipa->model, (Vector3){pipa->x, pipa->y, pipa->z},
              (Vector3){0, 1, 0}, angle, (Vector3){1, 1, 1}, WHITE);
}

struct Entity CreatePipa() {
  struct Entity pipa = {};
  pipa.model = LoadModel("pipa.glb");
  pipa.model.materials[2].maps[MATERIAL_MAP_DIFFUSE].texture = pipa_tex.texture;
  pipa.y = 10.0f;

  pipa.Update = &UpdatePipa;
  return pipa;
}

void UpdateBomba(struct Entity *bomba) {
  (*bomba).y += BOMBA_SPEED * GetFrameTime();
  if (bomba->y > 15.0f)
    (*bomba).dead = true;

  DrawModelEx(bomba->model, (Vector3){bomba->x, bomba->y, bomba->z},
              (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, WHITE);
}

struct Entity CreateBomba(float z_pos, struct Entity *next_entity) {
  struct Entity bomba = {};
  bomba.model = LoadModel("bomba.glb");
  bomba.y = -15.0f;
  bomba.z = z_pos;
  bomba.Update = &UpdateBomba;
  bomba.next_entity = next_entity;
  return bomba;
}

// void UpdateBullet(struct Entity *bullet);
// void UpdateBullet(struct Entity *bullet) {
//   (*bullet).x += BULLET_SPEED * GetFrameTime() * bullet->scaleX;
//   Rectangle source_rec = {bullet->texture.width, 0,
//                           bullet->texture.width * bullet->scaleX,
//                           bullet->texture.height};
//
//   Rectangle dest_rec = {bullet->x, bullet->y, bullet->texture.width,
//                         bullet->texture.height};
//
//   DrawTexturePro(bullet->texture, source_rec, dest_rec, (Vector2){0, 0}, 0,
//                  WHITE);
//
//   if (bullet->x > screenWidth || bullet->x < 0) {
//     (*bullet).dead = true;
//   }
// }
//
// struct Entity CreateBullet(int x, int y, int dir, struct Entity
// *next_entity); struct Entity CreateBullet(int x, int y, int dir, struct
// Entity *next_entity) {
//   struct Entity bullet = {};
//
//   bullet.x = x;
//   bullet.y = y;
//   bullet.scaleX = (float)dir;
//   bullet.scaleY = 1.0;
//   bullet.texture = bullet_texture;
//   bullet.next_entity = next_entity;
//   bullet.Update = &UpdateBullet;
//
//   return bullet;
// }
//
// void UpdatePlayer(struct Entity *player);
// void UpdatePlayer(struct Entity *player) {
//   Vector2 vec = {};
//
//   if (IsKeyDown(KEY_D) ||
//       IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
//     vec.x = 1;
//     (*player).scaleX = 1;
//   } else if (IsKeyDown(KEY_A) ||
//              IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
//     vec.x = -1;
//     (*player).scaleX = -1;
//   }
//
//   if (IsKeyDown(KEY_W) || IsGamepadButtonDown(0,
//   GAMEPAD_BUTTON_LEFT_FACE_UP)) {
//     vec.y = -1;
//   } else if (IsKeyDown(KEY_S) ||
//              IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
//     vec.y = 1;
//   }
//
//   if (vec.x != 0 || vec.y != 0) {
//     vec = Vector2Normalize(vec);
//   }
//
//   (*player).x += vec.x * PLAYER_SPEED * GetFrameTime();
//   (*player).y += vec.y * PLAYER_SPEED * GetFrameTime();
//
//   Rectangle source_rec = {player->texture.width, 0,
//                           player->texture.width * player->scaleX,
//                           player->texture.height};
//
//   Rectangle dest_rec = {player->x, player->y, player->texture.width,
//                         player->texture.height};
//
//   DrawTexturePro(player->texture, source_rec, dest_rec, (Vector2){0, 0}, 0,
//                  WHITE);
//   // DrawTexture(player->texture, player->x, player->y, WHITE);
//
//   if (IsKeyPressed(KEY_SPACE)) {
//     struct Entity *old_next = player->next_entity;
//     (*player).next_entity = malloc(sizeof(struct Entity));
//     *(*player).next_entity =
//         CreateBullet(player->x, player->y, player->scaleX, old_next);
//   }
// }
//
// struct Entity CreatePlayer();
// struct Entity CreatePlayer() {
//   struct Entity player = {};
//
//   player.texture = LoadTexture("boneco.png");
//   player.Update = &UpdatePlayer;
//   player.scaleX = 1.0;
//   player.scaleY = 1.0;
//
//   return player;
// }

void UpdateDrawFrame(struct Entity *first_entity);

//------------------ASSETS--------------

int main() {
#ifndef PLATFORM_WEB
  srand(time(NULL));
#else
  srand(emscripten_get_now());
#endif

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "Domestique");

#ifndef PLATFORM_ANDROID
  ChangeDirectory("assets");
#endif

  SetExitKey(0);

  SetTargetFPS(60);
  // bullet_texture = LoadTexture("bala.png");
  screen_tex = LoadRenderTexture(400, 300 * 2);
  pipa_tex = LoadRenderTexture(400, 300);
  BeginTextureMode(screen_tex);
  ClearBackground(WHITE);
  EndTextureMode();
  BeginTextureMode(pipa_tex);
  ClearBackground(WHITE);
  EndTextureMode();
  camera = (Camera3D){0};
  camera.position = (Vector3){35.0f, 0.0f, 0.0f}; // Camera position
  camera.target = (Vector3){0.0f, 0.0f, 0.0f};    // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE; // Camera projection type
  // pipa_model = LoadModel("pipa.glb");

  last_mouse_pos = (Vector2){0, 0};
  first_entity = malloc(sizeof(struct Entity));

  *first_entity = (struct Entity){};
  *first_entity = CreatePipa();

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
  // SetTargetFPS(60);
  // struct Entity *first_entity = (struct Entity *)malloc(sizeof(struct
  // player));

  // #ifndef PLATFORM_ANDROID
  // ChangeDirectory("..");
  // #endif
  while (!WindowShouldClose()) {
    UpdateDrawFrame(first_entity);
  }
#endif

  CloseWindow();
  return 0;
}

void UpdateEmscripten() { UpdateDrawFrame(first_entity); }

void DrawLineCircles(Vector2 start, Vector2 end, float radius, Color color) {
  float max = 20.0f;
  for (int i = 0; i < max; i++) {

    DrawCircle(Lerp(start.x, end.x, i / max), Lerp(start.y, end.y, i / max),
               radius, color);
  }
}

float timer = 0.0f;

void UpdateDrawFrame(struct Entity *first_entity) {
  if (game_state == 1) {
    timer += GetFrameTime();
    if (timer >= 2.0f) {
      struct Entity *bomba_pointer = malloc(sizeof(struct Entity));
      *bomba_pointer = CreateBomba(0.0f, first_entity->next_entity);
      (*first_entity).next_entity = bomba_pointer;
      timer = 0.0f;
    }
  }
  // Draw
  //----------------------------------------------------------------------------------
  BeginDrawing();
  ClearBackground(LIGHTGRAY);
  DrawFPS(10, 10);
  Vector3 cubePosition = {0.0f, 0.0f, 0.0f};
  BeginTextureMode(screen_tex);
  ClearBackground(RAYWHITE);
  BeginMode3D(camera);

  // DrawCube(cubePosition, 2.0f, 2.0f, 2.0f, RED);
  // DrawCubeWires(cubePosition, 2.0f, 2.0f, 2.0f, MAROON);
  // DrawModel(pipa_model, (Vector3){0, 10, 0}, 1, WHITE);

  struct Entity *entity = first_entity;
  struct Entity *previous_entity = NULL;
  while (entity != NULL) {
    if (entity->dead) {
      printf("Killing entity\n");
      struct Entity *next_entity = entity->next_entity;
      struct Entity *dead_entity = entity;
      if (first_entity != NULL && previous_entity != NULL) {
        (*previous_entity).next_entity = entity->next_entity;
        entity = entity->next_entity;
      } else {
        first_entity = next_entity;
      }
      free(dead_entity);
    } else {
      entity->Update(entity);
      previous_entity = entity;
      entity = entity->next_entity;
    }
  }

  EndMode3D();
  EndTextureMode();
  DrawTextureRec(
      screen_tex.texture,
      (Rectangle){0, screen_tex.texture.height / 2.0,
                  (float)screen_tex.texture.width,
                  (float)-screen_tex.texture.height / 2.0},
      (Vector2){screenWidth / 2.0 - screen_tex.texture.width / 2.0, 50}, WHITE);
  DrawTextureRec(
      screen_tex.texture,
      (Rectangle){0, 0, (float)screen_tex.texture.width,
                  (float)-screen_tex.texture.height / 2.0},
      (Vector2){screenWidth / 2.0 - screen_tex.texture.width / 2.0, 450},
      WHITE);
  if (game_state == 0) {
    float mouse_x =
        GetMouseX() - screenWidth / 2.0 + screen_tex.texture.width / 2.0;
    float mouse_y = GetMouseY() - 450;
    mouse_y = -(mouse_y - 300);
    if (IsMouseButtonPressed(0)) {
      last_mouse_pos = (Vector2){mouse_x, mouse_y};
    }
    if (IsMouseButtonDown(0)) {
      BeginTextureMode(pipa_tex);

      // DrawCircle(mouse_x, mouse_y, 10.0f, BLACK);
      DrawLineCircles(last_mouse_pos, (Vector2){mouse_x, mouse_y}, 10.0f,
                      BLACK);
      EndTextureMode();
    } else if (IsMouseButtonDown(1)) {
      BeginTextureMode(pipa_tex);
      DrawLineCircles(last_mouse_pos, (Vector2){mouse_x, mouse_y}, 10.0f,
                      WHITE);

      // DrawCircle(mouse_x, mouse_y, 10.0f, WHITE);
      EndTextureMode();
    }
    last_mouse_pos = (Vector2){mouse_x, mouse_y};

    DrawTextureRec(
        pipa_tex.texture,
        (Rectangle){0, 0, (float)pipa_tex.texture.width,
                    (float)pipa_tex.texture.height},
        (Vector2){screenWidth / 2.0 - pipa_tex.texture.width / 2.0, 450},
        WHITE);
    DrawText("DESENHE AQUI", 320, 460, 20, DARKGREEN);

    DrawText("APERTE ESPAÇO PARA COMEÇAR", 230, 720, 20, DARKGREEN);
    if (IsKeyPressed(KEY_SPACE)) {
      game_state = 1;
    }
  }

  EndDrawing();
}
