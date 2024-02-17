#include "lab_m1/lab5/lab5.h"

#include <random>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Lab5::Lab5()
{
}


Lab5::~Lab5()
{
}

struct inter_x { float lower; float upper; };
struct inter_z { float lower; float upper; };
std::vector<inter_x> excludedIntervalsx;
std::vector<inter_z> excludedIntervalsz;

bool isExcludedx(float value, const std::vector<inter_x>& excludedIntervals) {
    for (const auto& interval : excludedIntervals) {
        if (value >= interval.lower && value <= interval.upper) {
            return true;
        }
    }
    return false;
}

bool isExcludedz(float value, const std::vector<inter_z>& excludedIntervals) {
    for (const auto& interval : excludedIntervals) {
        if (value >= interval.lower && value <= interval.upper) {
            return true;
        }
    }
    return false;
}


void Lab5::Init()
{
    renderCameraTarget = false;
    camera = new implemented::Camera();
    tank_pos = glm::vec3(0, 0.25f, 0);
    angle = 0;
    angle_turret = 0;
    camera->Set(glm::vec3(tank_pos.x - 3 * cos(-angle), 1.5f, tank_pos.z - 3 * sin(-angle)),
        tank_pos,
        glm::vec3(0, 1, 0));
    nr_proiectils = 0;

    tank_npc[0] = glm::vec3(-10, 0.25f, 15);
    tank_npc[1] = glm::vec3(14, 0.25f, 9);
    tank_npc[2] = glm::vec3(16, 0.25f, -14);
    tank_npc[3] = glm::vec3(-13, 0.25f, -14);
    tank_npc[4] = glm::vec3(-30, 0.25f, 4);

    turbo_cooldown = 5;
    turbo_timer = 0;
    tank_coll = 0;
    speed = 3.f;
    excludedIntervalsx = {};
    excludedIntervalsz = {};
    excludedIntervalsx.push_back({tank_pos.x - 3.5f - 1.8f, tank_pos.x + 3.5f + 1.8f });
    excludedIntervalsz.push_back({ tank_pos.z - 3.5f - 1.8f, tank_pos.z + 3.5f + 1.8f });

    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < 4; i++) {
        std::uniform_real_distribution<float> dis(-30, 30);
        int x, z;
        do {
            x = dis(gen);
            z = dis(gen);
        } while (isExcludedx(x, excludedIntervalsx) && isExcludedz(z, excludedIntervalsz));
        bloc_pos[i] = glm::vec3(x, 5.f, z);
        excludedIntervalsx.push_back({x - 3.5f - 1.8f, x + 3.5f + 1.8f });
        excludedIntervalsz.push_back({z - 3.5f - 1.8f, z + 3.5f + 1.8f });
        do {
            x = dis(gen);
            z = dis(gen);
        } while (isExcludedx(x, excludedIntervalsx) && isExcludedz(z, excludedIntervalsz));
        excludedIntervalsx.push_back({ x - 3.5f, x + 3.5f });
        excludedIntervalsz.push_back({ z - 3.5f, z + 3.5f });
        tank_npc[i] = glm::vec3(x, 0.25f, z);
        
        npc_coll[i] = 0;
        action_timer[i] = rand() % 2;
        npc_action[i] = rand() % 4;
        npc_angle[i] = rand() % 5;
        angle_cannon_npc[i] = 0;
        nr_bullet_npc[i] = 1;
        bullet_coll[i][0] = false;
        timer_bullet[i][0] = 0.f;
    }
    game_timer = 120;
    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tank_wheels");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "tank_wheels.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tank_body");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "tank_body.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tank_turret");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "tank_turret.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Mesh* mesh = new Mesh("tank_cannon");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "tank_cannon.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }
    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab5", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "lab5", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, Z_NEAR, Z_FAR);
}


void Lab5::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Lab5::Update(float deltaTimeSeconds)
{
    if (game_timer >= 0 && tank_coll <7) {
        game_timer -= deltaTimeSeconds;
        for (int i = 0; i < nr_proiectils; i++) {
            timer_proiectils[i] -= deltaTimeSeconds;
            for (int j = 0; j < 4; j++) {
                float tank_dist = std::sqrt((proiectil[i].x + 6 * cos(-angle_cannon[i]) * deltaTimeSeconds - tank_npc[j].x) *
                    (proiectil[i].x + 6 * cos(-angle_cannon[i]) * deltaTimeSeconds - tank_npc[j].x) +
                    (proiectil[i].z + 6 * sin(-angle_cannon[i]) * deltaTimeSeconds - tank_npc[j].z) *
                    (proiectil[i].z + 6 * sin(-angle_cannon[i]) * deltaTimeSeconds - tank_npc[j].z));
                float x = std::max(bloc_pos[j].x - 3.5f, std::min(proiectil[i].x + 6 * cos(-angle_cannon[i]) * deltaTimeSeconds, bloc_pos[j].x + 3.5f));
                float z = std::max(bloc_pos[j].z - 3.5f, std::min(proiectil[i].z + 6 * sin(-angle_cannon[i]) * deltaTimeSeconds, bloc_pos[j].z + 3.5f));
                float bloc_dist = std::sqrt((x - (proiectil[i].x + 6 * cos(-angle_cannon[i]) * deltaTimeSeconds)) *
                    (x - (proiectil[i].x + 6 * cos(-angle_cannon[i]) * deltaTimeSeconds)) +
                    (z - (proiectil[i].z + 6 * sin(-angle_cannon[i]) * deltaTimeSeconds)) *
                    (z - (proiectil[i].z + 6 * sin(-angle_cannon[i]) * deltaTimeSeconds)));
                if (tank_dist < 1.85f && proiectil_coll[i] == false && npc_coll[j] < 7) {
                    proiectil_coll[i] = true;
                    npc_coll[j]++;
                }
                if (bloc_dist < 0.1f) {
                    proiectil_coll[i] = true;
                }
            }
            if (timer_proiectils[i] > 0 && proiectil_coll[i] == false) {
                glm::mat4 modelMatrix = glm::mat4(1);
                proiectil[i].z += 6 * sin(-angle_cannon[i]) * deltaTimeSeconds;
                proiectil[i].x += 6 * cos(-angle_cannon[i]) * deltaTimeSeconds;
                modelMatrix = glm::translate(modelMatrix, proiectil[i]);
                modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
                RenderMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(0), -10);
            }
        }
        for (int i = 0; i < 4; i++) {
            angle_cannon_npc[i] = -npc_angle[i];
            float tank_dist = std::sqrt((tank_npc[i].x - tank_pos.x) *
                (tank_npc[i].x - tank_pos.x) +
                (tank_npc[i].z - tank_pos.z) *
                (tank_npc[i].z - tank_pos.z));
            if (tank_dist < 8.f && npc_coll[i] < 7) {
                float x = tank_pos.x - tank_npc[i].x;
                float z = tank_pos.z - tank_npc[i].z;
                angle_cannon_npc[i] = atan2(z, x);
                if (timer_bullet[i][nr_bullet_npc[i] - 1] < 0) {
                    bullet_pos[i][nr_bullet_npc[i]].x = tank_npc[i].x + 1.4f * cos(angle_cannon_npc[i]);
                    bullet_pos[i][nr_bullet_npc[i]].y = tank_npc[i].y + 0.5957f;
                    bullet_pos[i][nr_bullet_npc[i]].z = tank_npc[i].z + 1.4f * sin(angle_cannon_npc[i]);
                    timer_bullet[i][nr_bullet_npc[i]] = 1.5f;
                    bullet_coll[i][nr_bullet_npc[i]] = false;
                    nr_bullet_npc[i]++;
                }

                for (int j = 0; j < nr_bullet_npc[i]; j++) {
                    timer_bullet[i][j] -= deltaTimeSeconds;
                    float dist = std::sqrt((bullet_pos[i][j].x + 6 * cos(angle_cannon_npc[i]) * deltaTimeSeconds - tank_pos.x)*
                        (bullet_pos[i][j].x + 6 * cos(angle_cannon_npc[i]) * deltaTimeSeconds - tank_pos.x)+
                        (bullet_pos[i][j].z + 6 * sin(angle_cannon_npc[i]) * deltaTimeSeconds - tank_pos.z)*
                        (bullet_pos[i][j].z + 6 * sin(angle_cannon_npc[i]) * deltaTimeSeconds - tank_pos.z));
                    if (dist < 1.9f && tank_coll < 7 && bullet_coll[i][j] == false) {
                        tank_coll++;
                        bullet_coll[i][j] = true;
                    }
                    if (timer_bullet[i][j] >= 0 && bullet_coll[i][j] == false) {
                        glm::mat4 modelMatrix = glm::mat4(1);
                        bullet_pos[i][j].z += 6 * sin(angle_cannon_npc[i]) * deltaTimeSeconds;
                        bullet_pos[i][j].x += 6 * cos(angle_cannon_npc[i]) * deltaTimeSeconds;
                        modelMatrix = glm::translate(modelMatrix, bullet_pos[i][j]);
                        modelMatrix = glm::rotate(modelMatrix, RADIANS(90.0f), glm::vec3(0, 1, 0));
                        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f));
                        RenderMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(0), -10);
                    }
                }
            }

        }

        //my tank
        turbo_cooldown -= deltaTimeSeconds;
        if (turbo_cooldown <= 0 && turbo_timer >= 0) {
            turbo_timer -= deltaTimeSeconds;
            speed = 4.5f;
        }
        if (turbo_cooldown <= 0 && speed == 4.5f && turbo_timer < 0) {
            speed = 3.f;
            turbo_cooldown = 2;
        }
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, tank_pos);
            modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
            glm::vec3 ghinion = glm::vec3(0, 0, 0);
            RenderMesh(meshes["tank_wheels"], shaders["LabShader"], modelMatrix, glm::vec3(1), -1000);
        }
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, tank_pos);
            modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
            RenderMesh(meshes["tank_body"], shaders["LabShader"], modelMatrix, glm::vec3(.9f, .8f, 1.f), -1000);
        }
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, tank_pos);
            modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, RADIANS(angle_turret), glm::vec3(0, 1, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
            RenderMesh(meshes["tank_turret"], shaders["LabShader"], modelMatrix, glm::vec3(.8f, .6f, 1.f), -1000);
        }
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, tank_pos);
            modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
            modelMatrix = glm::rotate(modelMatrix, RADIANS(angle_turret), glm::vec3(0, 1, 0));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
            RenderMesh(meshes["tank_cannon"], shaders["LabShader"], modelMatrix, glm::vec3(0.6f), -1000);
        }
        for (int i = 0; i < 4; i++) {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, bloc_pos[i]);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(7.f, 10.f, 7.f));
            RenderMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.7f, 0.7f, 0.7f), -10);
        }
        //npcs
        for (int i = 0; i < 4; i++) {
            float npc_speed = 1.5f;
            if (npc_coll[i] < 7) {
                action_timer[i] -= deltaTimeSeconds;
                if (action_timer[i] <= 0) {
                    action_timer[i] = rand() % 5;
                    npc_action[i] = rand() % 5;
                }

                if (npc_action[i] == 1) {
                    bool flag = true;
                    for (int j = 0; j < 4; j++) {
                        float x = std::max(bloc_pos[j].x - 3.5f, std::min(tank_npc[i].x + deltaTimeSeconds * npc_speed * cos(-npc_angle[i]), bloc_pos[j].x + 3.5f));
                        float z = std::max(bloc_pos[j].z - 3.5f, std::min(tank_npc[i].z + deltaTimeSeconds * npc_speed * sin(-npc_angle[i]), bloc_pos[j].z + 3.5f));
                        float dist = std::sqrt((x - (tank_npc[i].x + deltaTimeSeconds * npc_speed * cos(-npc_angle[i]))) *
                            (x - (tank_npc[i].x + deltaTimeSeconds * npc_speed * cos(-npc_angle[i]))) +
                            (z - (tank_npc[i].z + deltaTimeSeconds * npc_speed * sin(-npc_angle[i]))) *
                            (z - (tank_npc[i].z + deltaTimeSeconds * npc_speed * sin(-npc_angle[i]))));
                        float tank_dist = std::sqrt((tank_npc[i].x + deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_pos.x) *
                            (tank_npc[i].x + deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_pos.x) +
                            (tank_npc[i].z + deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_pos.z) *
                            (tank_npc[i].z + deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_pos.z));
                        float all_tank_dist = 3.8f;
                        if (j != i) {
                            all_tank_dist = std::sqrt((tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_npc[j].x) *
                                (tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_npc[j].x) +
                                (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_npc[j].z) *
                                (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_npc[j].z));
                        }
                        if (dist < 1.8f || all_tank_dist < 3.5f) {
                            flag = false;
                        }
                        else if (tank_dist < 3.5f) {
                            camera->MoveForward(deltaTimeSeconds * npc_speed);
                            camera->Set(glm::vec3(tank_pos.x - 3 * cos(-angle), 1.5f, tank_pos.z - 3 * sin(-angle)),
                                tank_pos,
                                glm::vec3(0, 1, 0));
                            tank_pos.x += deltaTimeSeconds * npc_speed * cos(-npc_angle[i]);
                            tank_pos.z += deltaTimeSeconds * npc_speed * sin(-npc_angle[i]);
                        }
                    }
                    if (flag == true) {
                        tank_npc[i].x += deltaTimeSeconds * npc_speed * cos(-npc_angle[i]);
                        tank_npc[i].z += deltaTimeSeconds * npc_speed * sin(-npc_angle[i]);
                    }
                }
                else if (npc_action[i] == 2) {
                    npc_angle[i] += deltaTimeSeconds;
                }
                else if (npc_action[i] == 3) {
                    bool flag = true;
                    for (int j = 0; j < 4; j++) {
                        float x = std::max(bloc_pos[j].x - 3.5f, std::min(tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]), bloc_pos[j].x + 3.5f));
                        float z = std::max(bloc_pos[j].z - 3.5f, std::min(tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]), bloc_pos[j].z + 3.5f));
                        float dist = std::sqrt((x - (tank_npc[i].x + deltaTimeSeconds * npc_speed * cos(-npc_angle[i]))) *
                            (x - (tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]))) +
                            (z - (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]))) *
                            (z - (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]))));
                        float tank_dist = std::sqrt((tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_pos.x) *
                            (tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_pos.x) +
                            (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_pos.z) *
                            (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_pos.z));
                        float all_tank_dist = 3.8f;
                        if (j != i) {
                            all_tank_dist = std::sqrt((tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_npc[j].x) *
                                (tank_npc[i].x - deltaTimeSeconds * npc_speed * cos(-npc_angle[i]) - tank_npc[j].x) +
                                (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_npc[j].z) *
                                (tank_npc[i].z - deltaTimeSeconds * npc_speed * sin(-npc_angle[i]) - tank_npc[j].z));
                        }
                        if (dist < 1.8f || all_tank_dist < 3.5f) {
                            flag = false;
                        }
                        else if (tank_dist < 3.5f) {
                            camera->MoveForward(deltaTimeSeconds * npc_speed);
                            camera->Set(glm::vec3(tank_pos.x - 3 * cos(-angle), 1.5f, tank_pos.z - 3 * sin(-angle)),
                                tank_pos,
                                glm::vec3(0, 1, 0));
                            tank_pos.x -= deltaTimeSeconds * npc_speed * cos(-npc_angle[i]);
                            tank_pos.z -= deltaTimeSeconds * npc_speed * sin(-npc_angle[i]);
                        }
                    }
                    if (flag == true) {
                        tank_npc[i].x -= deltaTimeSeconds * npc_speed * cos(-npc_angle[i]);
                        tank_npc[i].z -= deltaTimeSeconds * npc_speed * sin(-npc_angle[i]);
                    }
                }
                else if (npc_action[i] == 4) {
                    npc_angle[i] -= deltaTimeSeconds;
                }
                {
                    glm::mat4 modelMatrix = glm::mat4(1);
                    modelMatrix = glm::translate(modelMatrix, tank_npc[i]);
                    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
                    modelMatrix = glm::rotate(modelMatrix, npc_angle[i], glm::vec3(0, 1, 0));
                    glm::vec3 ghinion = glm::vec3(0, 0, 0);
                    RenderMesh(meshes["tank_wheels"], shaders["LabShader"], modelMatrix, glm::vec3(0.8f), i);
                }
                {
                    glm::mat4 modelMatrix = glm::mat4(1);
                    modelMatrix = glm::translate(modelMatrix, tank_npc[i]);
                    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
                    modelMatrix = glm::rotate(modelMatrix, npc_angle[i], glm::vec3(0, 1, 0));
                    RenderMesh(meshes["tank_body"], shaders["LabShader"], modelMatrix, glm::vec3(.0f, .4f, .0f), i);
                }
                {
                    glm::mat4 modelMatrix = glm::mat4(1);
                    modelMatrix = glm::translate(modelMatrix, tank_npc[i]);
                    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
                    modelMatrix = glm::rotate(modelMatrix, -angle_cannon_npc[i], glm::vec3(0, 1, 0));
                    RenderMesh(meshes["tank_turret"], shaders["LabShader"], modelMatrix, glm::vec3(.0f, .6f, .0f), i);
                }
                {
                    glm::mat4 modelMatrix = glm::mat4(1);
                    modelMatrix = glm::translate(modelMatrix, tank_npc[i]);
                    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f));
                    modelMatrix = glm::rotate(modelMatrix, -angle_cannon_npc[i], glm::vec3(0, 1, 0));
                    RenderMesh(meshes["tank_cannon"], shaders["LabShader"], modelMatrix, glm::vec3(0.8f), i);
                }
            }
        }
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(100.f));
            glm::vec3 ghinion = glm::vec3(0, 0, 0);
            RenderMesh(meshes["plane"], shaders["LabShader"], modelMatrix, glm::vec3(0.9f, 1.f, 0.8f), -10);
                }
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(100.f));
            glm::vec3 ghinion = glm::vec3(0, 0, 0);
            RenderMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.8f, 0.9f, 1.f), -10);
        }
    }
}


void Lab5::FrameEnd()
{
    //DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}


void Lab5::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color, int i)
{
    if (!mesh || !shader || !shader->program)
        return;
    // Render an object using the specified shader and the specified position
    shader->Use();
    GLint locObject = glGetUniformLocation(shader->program, "object_color");
    glUniform3fv(locObject, 1, glm::value_ptr(color));
    locObject = glGetUniformLocation(shader->program, "object_decolor");
    if (i >= 0) {
        const glm::vec3 decolor = glm::vec3(npc_coll[i] * 0.1f);
        glUniform3fv(locObject, 1, glm::value_ptr(decolor));
    }
    else if (i == -1000) {
        const glm::vec3 decolor = glm::vec3(tank_coll * 0.1f);
        glUniform3fv(locObject, 1, glm::value_ptr(decolor));
    }
    else {
        const glm::vec3 decolor = glm::vec3(0);
        glUniform3fv(locObject, 1, glm::value_ptr(decolor));
    }
    locObject = glGetUniformLocation(shader->program, "deformation_factor");
    if (i >= 0) {
        glUniform1i(locObject, npc_coll[i]);
    }
    else if (i == -1000) {
        glUniform1i(locObject, tank_coll);
    }
    else {
        glUniform1i(locObject, 0);
    }
    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    mesh->Render();
}


void Lab5::OnInputUpdate(float deltaTime, int mods)
{
    if (game_timer != 0 && tank_coll < 7) {
        if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
        {
            float cameraSpeed = 1.5f;
            float npc_speed = 1.5f;
            if (window->KeyHold(GLFW_KEY_W))
            {
                bool flag = true;
                for (int i = 0; i < 4; i++) {
                    float x = std::max(bloc_pos[i].x - 3.5f, std::min(tank_pos.x + deltaTime * speed * cos(-angle), bloc_pos[i].x + 3.5f));
                    float z = std::max(bloc_pos[i].z - 3.5f, std::min(tank_pos.z + deltaTime * speed * sin(-angle), bloc_pos[i].z + 3.5f));
                    float dist = std::sqrt((x - (tank_pos.x + deltaTime * speed * cos(-angle))) *
                        (x - (tank_pos.x + deltaTime * speed * cos(-angle))) +
                        (z - (tank_pos.z + deltaTime * speed * sin(-angle))) *
                        (z - (tank_pos.z + deltaTime * speed * sin(-angle))));
                    float tank_dist = std::sqrt((tank_pos.x + deltaTime * speed * cos(-angle) - tank_npc[i].x) *
                        (tank_pos.x + deltaTime * speed * cos(-angle) - tank_npc[i].x) +
                        (tank_pos.z + deltaTime * speed * sin(-angle) - tank_npc[i].z) *
                        (tank_pos.z + deltaTime * speed * sin(-angle) - tank_npc[i].z));
                    bool tank_tank = true;
                    for (int j = 0; j < 4; j++) {
                        float xx = std::max(bloc_pos[i].x - 3.5f, std::min(tank_npc[j].x + deltaTime * npc_speed * cos(-angle), bloc_pos[i].x + 3.5f));
                        float zz = std::max(bloc_pos[i].z - 3.5f, std::min(tank_npc[j].z + deltaTime * npc_speed * sin(-angle), bloc_pos[i].z + 3.5f));
                        float distz = std::sqrt((xx - (tank_npc[j].x + deltaTime * npc_speed * cos(-angle))) *
                            (xx - (tank_npc[j].x + deltaTime * npc_speed * cos(-angle))) +
                            (zz - (tank_npc[j].z + deltaTime * npc_speed * sin(-angle))) *
                            (zz - (tank_npc[j].z + deltaTime * npc_speed * sin(-angle))));
                        if (distz < 1.8f) {
                            tank_tank = false;
                        }
                    }
                    if (dist < 5.4f && tank_tank == false) {
                        flag = false;
                    }
                    else if (dist < 1.8f) {
                        flag = false;
                    }
                    else if (tank_dist < 3.5f && npc_coll[i] < 7) {
                        tank_npc[i].x += deltaTime * speed * cos(-angle);
                        tank_npc[i].z += deltaTime * speed * sin(-angle);
                    }
                }
                if (flag == true) {
                    camera->MoveForward(deltaTime * speed);
                    tank_pos.x += deltaTime * speed * cos(-angle);
                    tank_pos.z += deltaTime * speed * sin(-angle);
                    camera->Set(glm::vec3(tank_pos.x - 3 * cos(-angle), 1.5f, tank_pos.z - 3 * sin(-angle)),
                        tank_pos,
                        glm::vec3(0, 1, 0));
                }
            }
            if (window->KeyHold(GLFW_KEY_A))
            {
                angle += deltaTime;
                camera->Set(glm::vec3(tank_pos.x - 3 * cos(-angle), 1.5f, tank_pos.z - 3 * sin(-angle)),
                    tank_pos,
                    glm::vec3(0, 1, 0));
            }
            if (window->KeyHold(GLFW_KEY_S))
            {
                bool flag = true;
                for (int i = 0; i < 4; i++) {
                    float x = std::max(bloc_pos[i].x - 3.5f, std::min(tank_pos.x - deltaTime * speed * cos(-angle), bloc_pos[i].x + 3.5f));
                    float z = std::max(bloc_pos[i].z - 3.5f, std::min(tank_pos.z - deltaTime * speed * sin(-angle), bloc_pos[i].z + 3.5f));
                    float dist = std::sqrt((x - (tank_pos.x - deltaTime * speed * cos(-angle))) *
                        (x - (tank_pos.x - deltaTime * speed * cos(-angle))) +
                        (z - (tank_pos.z - deltaTime * speed * sin(-angle))) *
                        (z - (tank_pos.z - deltaTime * speed * sin(-angle))));
                    float tank_dist = std::sqrt((tank_pos.x - deltaTime * speed * cos(-angle) - tank_npc[i].x) *
                        (tank_pos.x - deltaTime * speed * cos(-angle) - tank_npc[i].x) +
                        (tank_pos.z - deltaTime * speed * sin(-angle) - tank_npc[i].z) *
                        (tank_pos.z - deltaTime * speed * sin(-angle) - tank_npc[i].z));
                    bool tank_tank = true;
                    for (int j = 0; j < 4; j++) {
                        float xx = std::max(bloc_pos[i].x - 3.5f, std::min(tank_npc[j].x - deltaTime * npc_speed * cos(-angle), bloc_pos[i].x + 3.5f));
                        float zz = std::max(bloc_pos[i].z - 3.5f, std::min(tank_npc[j].z - deltaTime * npc_speed * sin(-angle), bloc_pos[i].z + 3.5f));
                        float distz = std::sqrt((xx - (tank_npc[j].x - deltaTime * npc_speed * cos(-angle))) *
                            (xx - (tank_npc[j].x - deltaTime * npc_speed * cos(-angle))) +
                            (zz - (tank_npc[j].z - deltaTime * npc_speed * sin(-angle))) *
                            (zz - (tank_npc[j].z - deltaTime * npc_speed * sin(-angle))));
                        if (distz < 1.8f) {
                            tank_tank = false;
                        }
                    }
                    if (dist < 5.4f && tank_tank == false) {
                        flag = false;
                    }
                    else if (dist < 1.8f) {
                        flag = false;
                    }
                    else if (tank_dist < 3.5f && npc_coll[i] < 7) {
                        tank_npc[i].x -= deltaTime * speed * cos(-angle);
                        tank_npc[i].z -= deltaTime * speed * sin(-angle);
                    }
                }
                if (flag == true) {
                    camera->MoveForward(deltaTime * speed);
                    tank_pos.x -= deltaTime * speed * cos(-angle);
                    tank_pos.z -= deltaTime * speed * sin(-angle);
                    camera->Set(glm::vec3(tank_pos.x - 3 * cos(-angle), 1.5f, tank_pos.z - 3 * sin(-angle)),
                        tank_pos,
                        glm::vec3(0, 1, 0));
                }
            }
            if (window->KeyHold(GLFW_KEY_D))
            {
                angle -= deltaTime;
                camera->Set(glm::vec3(tank_pos.x - 3 * cos(-angle), 1.5f, tank_pos.z - 3 * sin(-angle)),
                    tank_pos,
                    glm::vec3(0, 1, 0));
            }
        }
        else {
            float CameraSpeed = 3.f;
            if (window->KeyHold(GLFW_KEY_A))
            {
                camera->RotateThirdPerson_OY(-deltaTime * CameraSpeed);
            }
            if (window->KeyHold(GLFW_KEY_D))
            {
                camera->RotateThirdPerson_OY(deltaTime * CameraSpeed);
            }
        }
    }
}


void Lab5::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_SPACE && turbo_cooldown <= 0) {
        turbo_timer = 5;
    }

}


void Lab5::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Lab5::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
    if (game_timer >= 0) {
        angle_turret -= deltaX * 0.1;
    }
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;
        renderCameraTarget = false;
        camera->RotateFirstPerson_OX(sensivityOX * -deltaY);
        camera->RotateFirstPerson_OY(sensivityOY * -deltaX);
    }
}


void Lab5::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event

    if (button == GLFW_MOUSE_BUTTON_2 && timer_proiectils[nr_proiectils - 1] <= 0.5f && game_timer >= 0) {
        proiectil[nr_proiectils].x = tank_pos.x + 1.4f * cos(-(angle + angle_turret / 64));
        proiectil[nr_proiectils].y = tank_pos.y + 0.5957f;
        proiectil[nr_proiectils].z = tank_pos.z + 1.4f * sin(-(angle + angle_turret / 64));
        angle_cannon[nr_proiectils] = angle_turret / 64 + angle;
        proiectil_coll[nr_proiectils] = false;
        timer_proiectils[nr_proiectils] = 1;
        nr_proiectils++;
    }

}


void Lab5::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Lab5::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Lab5::OnWindowResize(int width, int height)
{
}
