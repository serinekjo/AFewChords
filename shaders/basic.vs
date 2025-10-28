#version 330

uniform mat4 mod;
uniform mat4 proj;

layout (location = 0) in vec3 vsiPosition;
layout (location = 1) in vec3 vsiNormal;
layout (location = 2) in vec2 vsiTexCoord;
 
out vec2 vsoTexCoord;
out vec3 vsoNormal;
out vec4 vsoModPosition;

void main(void) {
  vsoNormal = (transpose(inverse(mod)) * vec4(vsiNormal.xyz, 0.0)).xyz;
  vsoModPosition = mod * vec4(vsiPosition.xyz, 1.0);
  gl_Position = proj * mod * vec4(vsiPosition.xyz, 1.0);
  vsoTexCoord = vec2(vsiTexCoord.x, 1.0 - vsiTexCoord.y);
}
