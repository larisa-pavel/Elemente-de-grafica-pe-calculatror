#pragma once

#include "components/simple_scene.h"
#include "lab_m1/lab5/lab_camera.h"

#define Z_FAR		(200.f)
#define Z_NEAR		(.01f)

namespace m1
{
    class Lab5 : public gfxc::SimpleScene
    {
     public:
        Lab5();
        ~Lab5();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, int i);

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

     protected:
        implemented::Camera *camera;
        glm::mat4 projectionMatrix;
        bool renderCameraTarget;

        // TODO(student): If you need any other class variables, define them here.
        GLfloat fov;
        bool projectionType;
        GLfloat right, left, bottom, top;
        GLfloat zNear = 0.1f, zFar = 60.f;
        float angle, angle_cannon[100], timer_proiectils[100], game_timer, angle_turret, turbo_timer, turbo_cooldown;
        int nr_proiectils, npc_coll[6], nr_bullet_npc[6], tank_coll;
        glm::vec3 tank_pos, offset_tank, bloc_pos[6], tank_npc[6];
        glm::vec3 proiectil[100], bullet_pos[6][100];
        bool proiectil_coll[100], bullet_coll[6][100];
        float timer_bullet[6][100];
        float speed, action_timer[6], npc_angle[6], npc_action[6], angle_bullet_npc[6][100], angle_cannon_npc[6];
    };
}   // namespace m1
