#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform sampler2D maskTexture;
uniform vec2 maskScale;

out vec4 finalColor;

void main() {
    vec2 maskCoord = fragTexCoord * maskScale;
    float maskValue = texture(maskTexture, maskCoord).r;
    
    if (maskValue > 0.5) {
        finalColor = texture(texture0, fragTexCoord) * fragColor;
    } else {
        discard;
    }
}

