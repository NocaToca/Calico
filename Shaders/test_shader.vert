
#version 450

layout(location = 1) in vec3 vertex_color;
layout(location = 0) in vec2 vertex_positions;
layout(location = 2) in vec2 tex_coords;

layout(location = 0) out vec3 frag_color;

void main(){

    gl_Position = vec4(vertex_positions, 0.0, 1.0);
    frag_color = vertex_color;

}