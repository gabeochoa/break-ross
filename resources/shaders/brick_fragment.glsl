#version 330

in vec2 fragUV;

uniform sampler2D healthTexture;
uniform ivec2 gridSize;
uniform float maxHealth;

out vec4 fragColor;

vec3 getHealthColor(float healthRatio) {
    if (healthRatio > 0.75) {
        return vec3(0.0, 1.0, 0.0);
    } else if (healthRatio > 0.5) {
        return vec3(1.0, 1.0, 0.0);
    } else if (healthRatio > 0.25) {
        return vec3(1.0, 0.65, 0.0);
    } else {
        return vec3(1.0, 0.0, 0.0);
    }
}

void main() {
    vec2 texCoord = (fragUV + 0.5) / vec2(gridSize);
    float health = texture(healthTexture, texCoord).r * maxHealth;
    
    if (health <= 0.0) {
        discard;
    }
    
    float healthRatio = health / maxHealth;
    vec3 color = getHealthColor(healthRatio);
    
    fragColor = vec4(color, 1.0);
}
