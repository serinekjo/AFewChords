#version 330

in float intensite_diffus;
in vec2 tc;


uniform vec4 couleur;
uniform sampler2D tex;
uniform bool is_disco;
uniform vec3 emission;

out vec4 fragColor;

void main() {
fragColor = couleur;
  vec4 texColor = texture(tex, tc);
  vec4 finalColor;
  if (is_disco) {
  finalColor = mix(texColor, intensite_diffus * couleur + vec4(1.0), 0.5);
  finalColor.rgb += emission;
} else {
  finalColor = texColor * couleur;
}


  fragColor = finalColor;
}
