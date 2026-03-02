#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "core/avatarEnemy.h"
#include <cstdio>
#include <cmath>

namespace AvatarSystem
{
  AvatarModel g_avatarModel;
}

//-----------------------------------------------------------------------------
// AvatarModel Implementation
//-----------------------------------------------------------------------------
bool AvatarModel::load(const char *path)
{
  // Check if file exists first
  FILE *testFile = fopen(path, "rb");
  if (!testFile)
  {
    printf("[Avatar] ERROR: File not found: %s\n", path);
    return false;
  }
  fclose(testFile);

  cgltf_options options = {};
  cgltf_data *data = nullptr;

  printf("[Avatar] Loading: %s\n", path);

  if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
  {
    printf("[Avatar] Failed to parse GLB: %s\n", path);
    return false;
  }

  if (cgltf_load_buffers(&options, data, path) != cgltf_result_success)
  {
    printf("[Avatar] Failed to load buffers: %s\n", path);
    cgltf_free(data);
    return false;
  }

  meshes.clear();

  for (size_t m = 0; m < data->meshes_count; m++)
  {
    cgltf_mesh *mesh = &data->meshes[m];
    for (size_t p = 0; p < mesh->primitives_count; p++)
    {
      cgltf_primitive *prim = &mesh->primitives[p];
      AvatarMesh amesh;

      // Extract vertex positions
      for (size_t a = 0; a < prim->attributes_count; a++)
      {
        cgltf_attribute *attr = &prim->attributes[a];
        if (attr->type == cgltf_attribute_type_position)
        {
          cgltf_accessor *acc = attr->data;
          amesh.positions.resize(acc->count * 3);
          for (size_t v = 0; v < acc->count; v++)
          {
            cgltf_accessor_read_float(acc, v, &amesh.positions[v * 3], 3);
          }
        }
      }

      // Extract indices
      if (prim->indices)
      {
        cgltf_accessor *idx = prim->indices;
        amesh.index_count = (int)idx->count;
        amesh.indices.resize(idx->count);
        for (size_t i = 0; i < idx->count; i++)
        {
          amesh.indices[i] = (unsigned int)cgltf_accessor_read_index(idx, i);
        }
      }

      if (amesh.index_count > 0)
        meshes.push_back(amesh);
    }
  }

  cgltf_free(data);
  loaded = !meshes.empty();

  if (loaded)
  {
    printf("[Avatar] Model loaded: %zu meshes\n", meshes.size());

    // Calculate bounding box to determine proper scale
    float minY = 1e9f, maxY = -1e9f;
    for (auto &mesh : meshes)
      for (size_t i = 1; i < mesh.positions.size(); i += 3)
      {
        minY = std::min(minY, mesh.positions[i]);
        maxY = std::max(maxY, mesh.positions[i]);
      }
    float height = maxY - minY;
    printf("[Avatar] Height in model units: %.2f (min=%.2f max=%.2f)\n", height, minY, maxY);
    printf("[Avatar] Recommended scale for 2.8 unit height: %.4f\n", 2.8f / height);

    for (size_t i = 0; i < meshes.size(); i++)
    {
      printf("[Avatar]   Mesh %zu: %d vertices, %d indices\n",
             i, (int)meshes[i].positions.size() / 3, meshes[i].index_count);
    }
  }
  else
    printf("[Avatar] Warning: No meshes found in %s\n", path);

  return loaded;
}

void AvatarModel::clear()
{
  meshes.clear();
  loaded = false;
}

namespace AvatarSystem
{
  //-----------------------------------------------------------------------------
  // AvatarSystem Implementation
  //-----------------------------------------------------------------------------
  bool loadModel(const char *path)
  {
    return g_avatarModel.load(path);
  }

  void clearModel()
  {
    g_avatarModel.clear();
  }

  float lookAtRotation(float fromX, float fromZ, float toX, float toZ)
  {
    float dx = toX - fromX;
    float dz = toZ - fromZ;
    return atan2f(dx, dz) * 180.0f / 3.14159265f;
  }

  void renderInstance(const AvatarEnemyInstance &inst)
  {
    if (!inst.active || !g_avatarModel.loaded)
      return;

    // Model is 2.64 units tall, make it MUCH BIGGER in world
    // Scale 10.0 = 26.4 units tall (giant enemy)
    const float MODEL_SCALE = 1.0f;
    const float Y_OFFSET = 0.0f;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();

    glTranslatef(inst.x, Y_OFFSET, inst.z);
    glRotatef(inst.rot_y, 0.0f, 1.0f, 0.0f);
    glScalef(MODEL_SCALE, MODEL_SCALE, MODEL_SCALE);

    // Kill every state that could interfere
    glUseProgram(0);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_FOG);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    bool isHurt = (inst.hurtTimer > 0.0f);

    for (const AvatarMesh &m : g_avatarModel.meshes)
    {
      if (m.index_count == 0 || m.positions.empty())
        continue;

      glEnableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_COLOR_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, m.positions.data());

      if (isHurt)
      {
        // Red flash
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3f(1.0f, 0.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, m.index_count,
                       GL_UNSIGNED_INT, m.indices.data());
      }
      else
      {
        // Pass 1 — solid black body
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 2.0f);
        glColor3f(0.0f, 0.0f, 0.0f); // Full black
        glDrawElements(GL_TRIANGLES, m.index_count,
                       GL_UNSIGNED_INT, m.indices.data());
        glDisable(GL_POLYGON_OFFSET_FILL);

        // Pass 2 — 96% black outline (almost black, very subtle)
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(1.5f);
        glColor3f(0.06f, 0.06f, 0.06f);
        glPushMatrix();
        glScalef(1.02f, 1.02f, 1.02f);
        glDrawElements(GL_TRIANGLES, m.index_count,
                       GL_UNSIGNED_INT, m.indices.data());
        glPopMatrix();
      }

      glDisableClientState(GL_VERTEX_ARRAY);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glPopMatrix();
    glPopAttrib();
  }

} // namespace AvatarSystem
