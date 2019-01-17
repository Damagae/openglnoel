#version 330

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;
uniform vec3 uKd;

uniform sampler2D uKdSampler;

in vec3 vFragPosition;
in vec3 vFragNormal;
in vec2 vFragTexCoords;

out vec3 fFragColor;

void main() {
    //fFragColor = normalize(vFragNormal);

    float distToPointLight = length(uPointLightPosition - vFragPosition);
    vec3 dirToPointLight = (uPointLightPosition - vFragPosition) / distToPointLight;
    vec4 tex = texture(uKdSampler, vFragTexCoords);
    fFragColor = vec3(tex.r, tex.g, tex.b) * uKd * (uDirectionalLightIntensity * max(0.0, dot(vFragNormal, uDirectionalLightDir))
                + uPointLightIntensity * max(0.0, dot(vFragNormal, dirToPointLight)) / (distToPointLight * distToPointLight));
};
