#version 330

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

in vec3 vFragPosition;
in vec3 vFragNormal;
in vec2 vFragTexCoords;

out vec3 fFragColor;

void main() {
    //fFragColor = normalize(vFragNormal);

    // float distToPointLight = length(uPointLightPosition - vFragPosition);
    // vec3 dirToPointLight = (uPointLightPosition - vFragPosition) / distToPointLight;
    // vec4 tex = texture(uKdSampler, vFragTexCoords);
    // fFragColor = vec3(tex.r, tex.g, tex.b) * uKd * (uDirectionalLightIntensity * max(0.0, dot(vFragNormal, uDirectionalLightDir))
    //             + uPointLightIntensity * max(0.0, dot(vFragNormal, dirToPointLight)) / (distToPointLight * distToPointLight));

    vec3 ka = uKa * vec3(texture(uKaSampler, vFragTexCoords));
    vec3 kd = uKd * vec3(texture(uKdSampler, vFragTexCoords));
    vec3 ks = uKs * vec3(texture(uKsSampler, vFragTexCoords));
    float shininess = uShininess * vec3(texture(uShininessSampler, vFragTexCoords)).x;

    vec3 normal = normalize(vFragNormal);
    vec3 eyeDir = normalize(-vFragPosition);

    float distToPointLight = length(uPointLightPosition - vFragPosition);
    vec3 dirToPointLight = (uPointLightPosition - vFragPosition) / distToPointLight;
    vec3 pointLightIncidentLight = uPointLightIntensity / (distToPointLight * distToPointLight);

    // half vectors, for blinn-phong shading
    vec3 hPointLight = normalize(eyeDir + dirToPointLight);
    vec3 hDirLight = normalize(eyeDir + uDirectionalLightDir);

    float dothPointLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hPointLight));
    float dothDirLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hDirLight));

    if (shininess != 1.f && shininess != 0.f)
    {
        dothPointLight = pow(dothPointLight, shininess);
        dothDirLight = pow(dothDirLight, shininess);
    }

    fFragColor = ka;
    fFragColor += kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + pointLightIncidentLight * max(0., dot(normal, dirToPointLight)));
    fFragColor += ks * (uDirectionalLightIntensity * dothDirLight + pointLightIncidentLight * dothPointLight);
};
