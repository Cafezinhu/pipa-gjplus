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

int points;
float difficulty;
const float DIFFICULTY_SCALE = .004;

const int screenWidth = 800;
const int screenHeight = 800;
const int PLAYER_SPEED = 10;
const int BOMBA_SPEED = 6;
Camera3D camera;
RenderTexture2D screen_tex;

RenderTexture2D pipa_tex;

Model pipa_model;
Model bomba_model;
Model foguete_model;
Model nave_model;

Texture bullet_texture;

Vector2 last_mouse_pos;

int game_state = 0;

struct Entity {
  struct Entity *next_entity;
  float x;
  float y;
  float z;
  float angle;
  float dir;
  Model model;
  bool dead;

  void (*Update)(struct Entity *player);
};

struct Entity *first_entity;

void UpdatePipa(struct Entity *pipa) {
  float angle = 0.0f;

  if (game_state == 1) {
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
  }

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
  if (game_state == 1) {
    (*bomba).y += BOMBA_SPEED * GetFrameTime() + difficulty;
    if (bomba->y > 19.0f) {
      difficulty += DIFFICULTY_SCALE;
      points += 1;

      (*bomba).dead = true;
    }
  } else if (game_state == 0) {
    (*bomba).dead = true;
  }

  DrawModelEx(bomba->model, (Vector3){bomba->x, bomba->y, bomba->z},
              (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, WHITE);
}

struct Entity CreateBomba(float z_pos, struct Entity *next_entity) {
  struct Entity bomba = {};
  bomba.model = bomba_model;
  bomba.y = -15.0f;
  bomba.z = z_pos;
  bomba.Update = &UpdateBomba;
  bomba.next_entity = next_entity;
  return bomba;
}

void UpdateFoguete(struct Entity *foguete) {
  if (game_state == 1) {
    (*foguete).y += BOMBA_SPEED * GetFrameTime() + difficulty;
    if (foguete->y > 19.0f) {
      (*foguete).dead = true;
      difficulty += DIFFICULTY_SCALE;
      points += 1;
    }
    (*foguete).z +=
        foguete->dir * BOMBA_SPEED * GetFrameTime() + difficulty * foguete->dir;
    if (foguete->z > 8) {
      (*foguete).z = 8.0f;
      (*foguete).dir = -1.0f;
    } else if (foguete->z < -8) {
      (*foguete).z = -8.0f;
      (*foguete).dir = 1.0f;
    }
  } else if (game_state == 0) {
    (*foguete).dead = true;
  }

  DrawModelEx(foguete->model, (Vector3){foguete->x, foguete->y, foguete->z},
              (Vector3){0, 1, 0}, 0.0f, (Vector3){1, 1, 1}, WHITE);
}

struct Entity CreateFoguete(float z_pos, struct Entity *next_entity) {
  struct Entity foguete = {};
  foguete.model = foguete_model;
  foguete.y = -15.0f;
  foguete.z = z_pos;
  foguete.dir = 1;
  foguete.Update = &UpdateFoguete;
  foguete.next_entity = next_entity;
  return foguete;
}

void UpdateNave(struct Entity *nave) {
  if (game_state == 1) {
    float mouse_x = (GetMouseX() - 400) / -21.0f;
    float mouse_y = (GetMouseY() - 400) / -26.0f;
    (*nave).z += nave->dir * BOMBA_SPEED * GetFrameTime() +
                 difficulty * nave->dir * 0.5f;

    if (Vector2Distance((Vector2){first_entity->z, 0.0f},
                        (Vector2){nave->z, 0.0f}) <= 1.0f) {
      game_state = 2;
    }
    if (Vector2Distance((Vector2){mouse_x, mouse_y},
                        (Vector2){nave->z, nave->y}) <= 5.0f &&
        IsMouseButtonPressed(0)) {
      (*nave).dead = true;
    }
    if (nave->z > 15.0f || nave->z < -15.0f)
      (*nave).dead = true;

  } else if (game_state == 0) {
    (*nave).dead = true;
  }

  DrawModelEx(nave_model, (Vector3){nave->x, nave->y, nave->z},
              (Vector3){0, 0, 0}, 0.0f, (Vector3){2, 2, 2}, WHITE);

  DrawCylinderEx((Vector3){nave->x, nave->y, nave->z},
                 (Vector3){nave->x, 20.0f, nave->z}, 0.2f, 0.2f, 8, RED);
}

struct Entity CreateNave(float z_pos, struct Entity *next_entity) {
  struct Entity nave = {};
  nave.y = -10.0f;
  nave.z = z_pos / fabs(z_pos) * 10.0f;
  nave.dir = -z_pos / fabs(z_pos);
  nave.Update = &UpdateNave;
  nave.next_entity = next_entity;
  return nave;
}

void UpdateDrawFrame(struct Entity *first_entity);
void UpdateEmscripten();

//------------------ASSETS--------------

int main() {
#ifndef PLATFORM_WEB
  srand(time(NULL));
#else
  srand(emscripten_get_now());
#endif

  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(screenWidth, screenHeight, "DS Pipa");

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
  first_entity = (struct Entity *)malloc(sizeof(struct Entity));

  *first_entity = (struct Entity){};
  *first_entity = CreatePipa();

  bomba_model = LoadModel("bomba.glb");
  foguete_model = LoadModel("foguete.glb");
  nave_model = LoadModel("nave.glb");

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(UpdateEmscripten, 0, 1);
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
    if (timer >= 1.3f - difficulty * 3.0f) {
      struct Entity *bomba_pointer =
          (struct Entity *)malloc(sizeof(struct Entity));
      float random_inimigo = rand() % 100;
      printf("random inimigo: %.3f\n", random_inimigo);

      if (random_inimigo < 25 && random_inimigo >= 12)
        *bomba_pointer =
            CreateFoguete(rand() % 16 - 8, first_entity->next_entity);
      else if (random_inimigo < 12)
        *bomba_pointer = CreateNave(rand() % 16 - 8, first_entity->next_entity);
      else
        *bomba_pointer =
            CreateBomba(rand() % 16 - 8, first_entity->next_entity);
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

  struct Entity *entity = first_entity;
  struct Entity *previous_entity = NULL;
  while (entity != NULL) {
    if (entity->dead) {
      // printf("Killing entity\n");
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
      if (entity != first_entity && game_state == 1) {
        // TODO: colisao
        Vector2 entity_pos = {entity->z, entity->y};
        Vector2 pipa_pos = {first_entity->z, first_entity->y};

        float distance = Vector2Distance(entity_pos, pipa_pos);
        if (distance <= 2.0f) {
          printf("COLISAO\n");
          game_state = 2;
        }
      }
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
  } else if (game_state == 2) {
    DrawText("APERTE ESPAÇO PARA REINICIAR", 230, 720, 20, DARKGREEN);

    if (IsKeyPressed(KEY_SPACE)) {
      game_state = 0;
      points = 0;
      difficulty = 0;
    }
  }

  if (game_state > 0) {
    char points_text[30];
    sprintf(points_text, "SCORE: %d", points);
    DrawText(points_text, 210, 60, 20, DARKGREEN);
  }

  EndDrawing();
}
