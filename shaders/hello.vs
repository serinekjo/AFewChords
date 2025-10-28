#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 mod;

uniform float tex_scale;
uniform vec2 tex_translate;

out float intensite_diffus;
out vec2 tc;

const vec3 L = vec3(0.0, 0.0, -1.0);

void main() {
  vec4 n = transpose(inverse(mod)) * vec4(normal, 0.0);
  intensite_diffus = clamp(dot(normalize(n.xyz), -L), 0, 1);
  gl_Position = proj * view * mod * vec4(pos, 1.0);
  tc = texCoord * tex_scale + tex_translate;
}
