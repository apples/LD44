#pragma once

#include <sushi/sushi.hpp>

class basic_shader_program : public sushi::shader_base {
public:
    basic_shader_program() = default;

    basic_shader_program(const std::string& vert, const std::string& frag);

    void set_MVP(const glm::mat4& mat);
    void set_normal_mat(const glm::mat4& mat);
    void set_s_texture(GLint i);
    void set_cam_forward(const glm::vec3& vec);
    void set_tint(const glm::vec4& v);
    void set_hue(float f);
    void set_saturation(float f);
    void set_sky_dir(const glm::vec3& vec);
    void set_sky_color(const glm::vec3& vec);
    void set_ambient_color(const glm::vec3& vec);
    void set_enable_lighting(bool b);

private:
    struct {
        GLint MVP;
        GLint normal_mat;
        GLint s_texture;
        GLint cam_forward;
        GLint tint;
        GLint hue;
        GLint saturation;
        GLint sky_dir;
        GLint sky_color;
        GLint ambient_color;
        GLint enable_lighting;
    } uniforms;
};

class msdf_shader_program : public sushi::shader_base {
public:
    msdf_shader_program() = default;

    msdf_shader_program(const std::string& vertfile, const std::string& fragfile);

    void set_MVP(const glm::mat4& mat);
    void set_normal_mat(const glm::mat4& mat);
    void set_msdf(int slot);
    void set_pxRange(float f);
    void set_texSize(const glm::vec2& vec);
    void set_fgColor(const glm::vec4& vec);

private:
    struct {
        GLint MVP;
        GLint normal_mat;
        GLint msdf;
        GLint pxRange;
        GLint texSize;
        GLint fgColor;
    } uniforms;
};
