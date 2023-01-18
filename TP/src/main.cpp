#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <Framebuffer.h>
#include <GLFW/glfw3.h>
#include <ImGuiRenderer.h>
#include <SceneView.h>
#include <Texture.h>
#include <graphics.h>
#include <imgui/imgui.h>
#include <iostream>
#include <vector>
#include <algorithm>

#define DATA_PATH "../../data/"

const std::array<std::string, 4> SCENES = { "sponza.glb", "forest_huge.glb", "box.glb", "cube.glb" };
std::string selected_scene = SCENES[0];
glm::mat4 last_proj_matrix;
glm::mat4 last_view_matrix;

using namespace OM3D;

static float delta_time = 0.0f;
const glm::uvec2 window_size(1600, 900);

void glfw_check(bool cond)
{
    if (!cond)
    {
        const char *err = nullptr;
        glfwGetError(&err);
        std::cerr << "GLFW error: " << err << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void update_delta_time()
{
    static double time = 0.0;
    const double new_time = program_time();
    delta_time = float(new_time - time);
    time = new_time;
}

void process_inputs(GLFWwindow *window, Camera &camera)
{
    static glm::dvec2 mouse_pos;

    glm::dvec2 new_mouse_pos;
    glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);

    {
        glm::vec3 movement = {};
        if (glfwGetKey(window, 'W') == GLFW_PRESS)
        {
            movement += camera.forward();
        }
        if (glfwGetKey(window, 'S') == GLFW_PRESS)
        {
            movement -= camera.forward();
        }
        if (glfwGetKey(window, 'D') == GLFW_PRESS)
        {
            movement += camera.right();
        }
        if (glfwGetKey(window, 'A') == GLFW_PRESS)
        {
            movement -= camera.right();
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            movement += camera.up();
        }

        float speed = camera.get_speed() * 100.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            speed *= 2.0f;
        }

        if (movement.length() > 0.0f)
        {
            const glm::vec3 new_pos =
                camera.position() + movement * delta_time * speed;
            camera.set_view(
                glm::lookAt(new_pos, new_pos + camera.forward(), camera.up()));
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        const glm::vec2 delta = glm::vec2(mouse_pos - new_mouse_pos) * camera.get_sensitivity();
        if (delta.length() > 0.0f)
        {
            glm::mat4 rot = glm::rotate(glm::mat4(1.0f), delta.x,
                                        glm::vec3(0.0f, 1.0f, 0.0f));
            rot = glm::rotate(rot, delta.y, camera.right());
            camera.set_view(glm::lookAt(
                camera.position(),
                camera.position() + (glm::mat3(rot) * camera.forward()),
                (glm::mat3(rot) * camera.up())));
        }
    }

    mouse_pos = new_mouse_pos;
}

std::unique_ptr<Scene> create_default_scene()
{
    auto scene = std::make_unique<Scene>();

    // Load default cube model
    auto result = Scene::from_gltf(std::string(data_path) + "sponza.glb", {});
    ALWAYS_ASSERT(result.is_ok, "Unable to load default scene");
    scene = std::move(result.value);

    // Add lights
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, 4.0f));
        light.set_color(glm::vec3(0.0f, 10.0f, 0.0f));
        light.set_radius(100.0f);
        scene->add_object(std::move(light));
    }
    {
        PointLight light;
        light.set_position(glm::vec3(1.0f, 2.0f, -4.0f));
        light.set_color(glm::vec3(10.0f, 0.0f, 0.0f));
        light.set_radius(50.0f);
        scene->add_object(std::move(light));
    }

    return scene;
}

void load_scene(std::string sceneName, std::unique_ptr<Scene>& scene, SceneView& scene_view, std::vector<std::string> debugDefines)
{
	last_view_matrix = scene_view.camera().view_matrix(); // save view matrix to restore it on scene load
    auto result = Scene::from_gltf(DATA_PATH + sceneName, debugDefines);
    if (!result.is_ok)
        std::cerr << "Unable to load scene (" << sceneName << ")" << std::endl;
    else
    {
		std::cout << "Loaded scene " << sceneName << " successfully." << std::endl;
        scene = std::move(result.value);
        scene_view = SceneView(scene.get());
		scene_view.camera().set_view(last_view_matrix);
    }
}

void update_selected_defines(std::vector<std::string>& selectedDefines, std::string define, bool condition)
{
    if (condition)
        selectedDefines.push_back(define);
    else
        selectedDefines.erase(std::remove(selectedDefines.begin(), selectedDefines.end(), define), selectedDefines.end());
}

void geometry_pass(Framebuffer& gbuffer, std::shared_ptr<Program> gbuffer_program, SceneView& scene_view)
{
    gbuffer.bind();
    gbuffer_program->bind();
    scene_view.render();
}

void light_pass(Texture& albedo, Texture& normal, Framebuffer& gbuffer, std::shared_ptr<Program> gbuffer_program)
{
    // glBindFramebuffer(GL_FRAMEBUFFER, 0); // default screen
}

int main(int, char **)
{
    DEBUG_ASSERT([] {
        std::cout << "Debug asserts enabled" << std::endl;
        return true;
    }());

    glfw_check(glfwInit());
    DEFER(glfwTerminate());

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(window_size.x, window_size.y,
                                          "Foward+ Renderer", nullptr, nullptr);
    glfw_check(window);
    DEFER(glfwDestroyWindow(window));

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    init_graphics();

    ImGuiRenderer imgui(window);

    std::unique_ptr<Scene> scene = create_default_scene();
    SceneView scene_view(scene.get());

    auto tonemap_program = Program::from_file("tonemap.comp");
    auto gbuffer_program = Program::from_files("gbuffer.frag", "basic.vert");
    // auto debug_gbuffer = Program::from_files("gbuffer.frag", "screen.vert");

    // GUI Variables
    bool enableTonemapping = true;
    bool enableDeferred = false;

    bool debugNormals = false;
    bool debugPosition = false;
    bool debugUV = false;

    std::vector<std::string> selectedDefines;

    // main framebuffer
    Texture lit(window_size, ImageFormat::RGBA16_FLOAT);
    Texture depth(window_size, ImageFormat::Depth32_FLOAT);
    Framebuffer main_framebuffer(&depth, std::array{ &lit });
    
    // tonemap framebuffer
    Texture color(window_size, ImageFormat::RGBA8_UNORM);
    Framebuffer tonemap_framebuffer(nullptr, std::array{ &color });
    
    // g-buffer
    Texture albedo(window_size, ImageFormat::RGBA8_UNORM); // no RGBA8_sRGB because bugs
    Texture normal(window_size, ImageFormat::RGBA8_UNORM);
    Framebuffer gbuffer(&depth, std::array{ &albedo, &normal });

    for (;;)
    {
        glfwPollEvents();
        if (glfwWindowShouldClose(window)
            || glfwGetKey(window, GLFW_KEY_ESCAPE))
        {
            break;
        }

        update_delta_time();

        if (const auto &io = ImGui::GetIO();
            !io.WantCaptureMouse && !io.WantCaptureKeyboard)
        {
            process_inputs(window, scene_view.camera());
        }

        // Render the scene
        if (enableDeferred)
        {
            geometry_pass(gbuffer, gbuffer_program, scene_view);
            light_pass(albedo, normal, gbuffer, gbuffer_program);
        }
        else
        {
            main_framebuffer.bind();
            scene_view.render();
        }

        if (enableTonemapping)
        {
            // Apply a tonemap in compute shader
            {
                tonemap_program->bind();
                lit.bind(0);
                color.bind_as_image(1, AccessType::WriteOnly);
                glDispatchCompute(align_up_to(window_size.x, 8),
                                  align_up_to(window_size.y, 8), 1);
            }
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (enableTonemapping)
            tonemap_framebuffer.blit();
        else if (enableDeferred)
            gbuffer.blit();
        else
			main_framebuffer.blit();
		
        
        // GUI
        imgui.start();
        {
            // Scene loading
            ImGui::Text("Scene loading");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            
            char buffer[1024] = {};
            if (ImGui::InputText("Load scene", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {
                load_scene(buffer, scene, scene_view, selectedDefines);
            }
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

            for (const std::string scene_name : SCENES)
            {
                ImGui::Button(scene_name.c_str());
                ImGui::SameLine();

                if (ImGui::IsItemClicked())
                {
                    selected_scene = scene_name;
                    load_scene(scene_name, scene, scene_view, selectedDefines);
                }
            }

            // Render features
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Separator();
			ImGui::Text("Render");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            
            ImGui::Checkbox("Tonemapping", &enableTonemapping);
            ImGui::Checkbox("Deferred rendering", &enableDeferred);

            if (ImGui::Checkbox("Show normals", &debugNormals))
            {
                update_selected_defines(selectedDefines, "DEBUG_NORMAL", debugNormals);
                load_scene(selected_scene, scene, scene_view, selectedDefines);
            }

            if (ImGui::Checkbox("Show position", &debugPosition))
            {
                update_selected_defines(selectedDefines, "DEBUG_POSITION", debugPosition);
                load_scene(selected_scene, scene, scene_view, selectedDefines);
            }

            if (ImGui::Checkbox("Show UV", &debugUV))
            {
                update_selected_defines(selectedDefines, "DEBUG_UV", debugUV);
                load_scene(selected_scene, scene, scene_view, selectedDefines);
            }

            // Camera
            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Separator();
			ImGui::Text("Camera");
            ImGui::Dummy(ImVec2(0.0f, 5.0f));

			ImGui::SliderFloat("Speed", scene_view.camera().get_speed_ptr(), 0.0f, 20.0f);
			ImGui::SliderFloat("Sensitivity", scene_view.camera().get_sensitivity_ptr(), 0.001f, 0.03f);
        }
        imgui.finish();

        glfwSwapBuffers(window);
    }

    scene = nullptr; // destroy scene and child OpenGL objects
}
