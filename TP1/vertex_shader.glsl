#version 330 core

layout(location = 0) in vec3 vertices_position_modelspace;
layout(location = 1) in vec2 aTexCoord;
out vec2 TexCoord;

uniform mat4 mvp;
uniform sampler2D heightMap;
uniform mat4 rotationMatrix;
uniform bool orbital;

void main() {

    // if(orbital){
    //     vec4 rotatedPosition = rotationMatrix * vec4(vertexPosition_modelspace, 1.0);
    //     gl_Position = mvp * rotatedPosition;
    //     TexCoord = aTexCoord;

    // }
    // else{

        vec3 pos=vertices_position_modelspace;
        pos.y=texture(heightMap, aTexCoord).x/3;
        
        gl_Position = mvp * vec4(pos, 1.0);



        TexCoord = aTexCoord;
    // }


}