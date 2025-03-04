#version 330 core
in vec2 TexCoord;
out vec3 color;

uniform sampler2D textureGrass;
uniform sampler2D textureRock;
uniform sampler2D textureNeige;
uniform sampler2D heightMap;


void main() {
    float heightValue = texture(heightMap, TexCoord).r;
    if (heightValue > 0.9) {
        color = texture(textureNeige, TexCoord).rgb;
    } 
    else if (heightValue <= 0.4) {
        color = texture(textureGrass, TexCoord).rgb;
    } 
    else if (heightValue <= 0.6) {
        float blend = heightValue;
        blend = clamp(blend, 0.0, 1.0);
        color = mix(texture(textureGrass, TexCoord).rgb, texture(textureRock, TexCoord).rgb, (heightValue-0.4)/0.2);
    } 
    else if (heightValue <= 0.8) {
        color = texture(textureRock, TexCoord).rgb;
    } 
    else if (heightValue <= 0.9) {
        float blend = heightValue ;
        blend = clamp(blend, 0.0, 1.0);
        color = mix(texture(textureRock, TexCoord).rgb, texture(textureNeige, TexCoord).rgb, (heightValue-0.8)/0.1);
    }


    


}
