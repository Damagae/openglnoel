#version 330

in vec3 vFragPosition;
in vec3 vFragNormal;
in vec2 vFragTexCoords;

layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

void main() {

    fAmbient = uKa * vec3(texture(uKaSampler, vFragTexCoords));
    fDiffuse = uKd * vec3(texture(uKdSampler, vFragTexCoords));
    vec3 ks = uKs * vec3(texture(uKsSampler, vFragTexCoords));
    float shininess = uShininess * vec3(texture(uShininessSampler, vFragTexCoords)).x;
    fGlossyShininess = vec4(ks, shininess);

    fNormal = normalize(vFragNormal);
    fPosition = vFragPosition;
};
