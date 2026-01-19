#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNK_SIZE 24
char* floatToStr(float value);
Mesh GenerateChunkMesh(int voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]);

int main(void){

    InitWindow(1400, 1080, "game window");
    SetTargetFPS(60);
    Camera3D cam = {0};
    //cam.position = (Vector3){0, 2, 5};   
    //cam.target   = (Vector3){0, 0, 0};   
    cam.position = (Vector3){CHUNK_SIZE/2.0f, CHUNK_SIZE, CHUNK_SIZE*2};
    cam.target   = (Vector3){CHUNK_SIZE/2.0f, CHUNK_SIZE/2.0f, CHUNK_SIZE/2.0f};
    cam.up       = (Vector3){0, 1, 0};
    cam.fovy     = 60;
    cam.projection = CAMERA_PERSPECTIVE;
    DisableCursor();

    int voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};
    // Fill some voxels for testing
    for(int x=0;x<CHUNK_SIZE;x++)
        for(int y=0;y<CHUNK_SIZE;y++)
            for(int z=0;z<CHUNK_SIZE;z++)
                if(y<CHUNK_SIZE) voxels[x][y][z] = 1;

    Mesh chunkMesh = GenerateChunkMesh(voxels);
    UploadMesh(&chunkMesh, false);
    Model chunkModel = LoadModelFromMesh(chunkMesh);

    while(!WindowShouldClose()){
        UpdateCamera(&cam, CAMERA_FIRST_PERSON); 
        char* fps = floatToStr(1/GetFrameTime());
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode3D(cam);
        
        DrawModel(chunkModel, (Vector3){0,12,0}, 1.0f, WHITE);
        DrawGrid(100, 1.0f);
        EndMode3D();
        DrawText(fps, 1, 1, 25, BLUE);
        free(fps);
        EndDrawing();
    }
    UnloadModel(chunkModel);
    CloseWindow();
    printf("success!");
}

char* floatToStr(float value) {
    char* str = malloc(32);
    if (!str) return NULL;
    snprintf(str, 32, "%g", value);
    return str;
}

//================ VIBE CODED AF FIX SOON=====EDIT====IT IS FIXED NOW LEARN HOW IT WORKS BETTER BEFORE MAKING MORE CHUNKS

static inline int voxelExists(int voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE], int x, int y, int z) {
    if(x<0 || x>=CHUNK_SIZE || y<0 || y>=CHUNK_SIZE || z<0 || z>=CHUNK_SIZE) return 0;
    return voxels[x][y][z] != 0;
}

const int dx[6] = {  1, -1,  0,  0,  0,  0 };
const int dy[6] = {  0,  0,  1, -1,  0,  0 };
const int dz[6] = {  0,  0,  0,  0,  1, -1 };

static const float faceNormals[6][3] = {
    {  1,  0,  0 }, // +X
    { -1,  0,  0 }, // -X
    {  0,  1,  0 }, // +Y
    {  0, -1,  0 }, // -Y
    {  0,  0,  1 }, // +Z
    {  0,  0, -1 }  // -Z
};

const float cubeVertices[24][3] = {
    // +X
    {1,0,0},{1,1,0},{1,1,1},{1,0,1},
    // -X
    {0,0,1},{0,1,1},{0,1,0},{0,0,0},
    // +Y
    {0,1,1},{1,1,1},{1,1,0},{0,1,0},
    // -Y
    {0,0,0},{1,0,0},{1,0,1},{0,0,1},
    // +Z
    {1,0,1},{1,1,1},{0,1,1},{0,0,1},
    // -Z
    {0,0,0},{0,1,0},{1,1,0},{1,0,0}
};

const unsigned short faceIndices[6] = {
    0, 1, 2,
    0, 2, 3
};


Mesh GenerateChunkMesh(int voxels[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]) {
    int maxVertices = 24 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    int maxIndices  = 36 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

    Mesh mesh = {0};
    mesh.vertexCount = 0;
    mesh.triangleCount = 0;
    mesh.normals = malloc(maxVertices * 3 * sizeof(float));

    mesh.vertices = malloc(maxVertices * 3 * sizeof(float));
    mesh.texcoords = malloc(maxVertices * 2 * sizeof(float));
    mesh.indices  = malloc(maxIndices * sizeof(unsigned short));
    mesh.colors   = malloc(maxVertices * 4 * sizeof(unsigned char));

    int vertexOffset = 0;
    int indexOffset = 0;

    for(int x=0; x<CHUNK_SIZE; x++) {
        for(int y=0; y<CHUNK_SIZE; y++) {
            for(int z=0; z<CHUNK_SIZE; z++) {
                if(!voxels[x][y][z]) continue;
                for(int f=0; f<6; f++) {
                    int nx = x + dx[f];
                    int ny = y + dy[f];
                    int nz = z + dz[f];

                    if(voxelExists(voxels, nx, ny, nz)) continue;

                    for(int v=0; v<4; v++) {
                        mesh.vertices[(vertexOffset+v)*3 + 0] = cubeVertices[f*4 + v][0] + x;
                        mesh.vertices[(vertexOffset+v)*3 + 1] = cubeVertices[f*4 + v][1] + y;
                        mesh.vertices[(vertexOffset+v)*3 + 2] = cubeVertices[f*4 + v][2] + z;

                        mesh.texcoords[(vertexOffset+v)*2 + 0] = (v==0||v==3)?0.0f:1.0f;
                        mesh.texcoords[(vertexOffset+v)*2 + 1] = (v<2)?1.0f:0.0f;

                        mesh.colors[(vertexOffset+v)*4 + 0] = WHITE.r;
                        mesh.colors[(vertexOffset+v)*4 + 1] = WHITE.g;
                        mesh.colors[(vertexOffset+v)*4 + 2] = WHITE.b;
                        mesh.colors[(vertexOffset+v)*4 + 3] = WHITE.a;

                        int n = (vertexOffset + v) * 3;
                        mesh.normals[n + 0] = faceNormals[f][0];
                        mesh.normals[n + 1] = faceNormals[f][1];
                        mesh.normals[n + 2] = faceNormals[f][2];
                    }

                    for(int t=0; t<6; t++) {
                        mesh.indices[indexOffset+t] = vertexOffset + faceIndices[t];
                    }

                    vertexOffset += 4;
                    indexOffset  += 6;
                }
            }
        }
    }
//sanity checking code here
    mesh.vertexCount = vertexOffset;
    mesh.triangleCount = indexOffset/3;
    printf("Vertices: %d, Triangles: %d\n", mesh.vertexCount, mesh.triangleCount);
    float minY = 1e9f, maxY = -1e9f;
    for (int i = 0; i < mesh.vertexCount; i++) {
    float y = mesh.vertices[i*3 + 1];
    if (y < minY) minY = y;
    if (y > maxY) maxY = y;
    }
    printf("Y range: %f -> %f (height = %f)\n",
       minY, maxY, maxY - minY);

    return mesh;
}
