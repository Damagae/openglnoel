#version 330

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGGlossyShininess;

uniform mat4 uDirLightViewProjMatrix;
uniform sampler2D uDirLightShadowMap;
uniform float uDirLightShadowMapBias;

out vec3 fFragColor;

void main() {
    vec3 GPosition = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0));
    vec3 normal = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0));

    vec3 ka = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0));
    vec3 kd = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0));
    vec4 ksShininess = texelFetch(uGGlossyShininess, ivec2(gl_FragCoord.xy), 0);
    vec3 ks = ksShininess.rgb;
    float shininess = ksShininess.a;

    vec3 position = normalize(-GPosition);

    float distToPointLight = length(uPointLightPosition - GPosition);
    vec3 dirToPointLight = (uPointLightPosition - GPosition) / distToPointLight;
    vec3 pointLightIncidentLight = uPointLightIntensity / (distToPointLight * distToPointLight);

    vec4 positionInDirLightScreen = uDirLightViewProjMatrix * vec4(position, 1); // applique la matrice pour projeter la position du fragment courant
    vec3 positionInDirLightNDC = vec3(positionInDirLightScreen / positionInDirLightScreen.w) * 0.5 + 0.5; // homogénise cette position projetée en divisant par la coordonnée "w" -entre 0 et 1
    float depthBlockerInDirSpace = texture(uDirLightShadowMap, positionInDirLightNDC.xy).r; //  lit la depth stockée dans la shadow map à la position du fragment
    float dirLightVisibility = positionInDirLightNDC.z < depthBlockerInDirSpace + uDirLightShadowMapBias ? 1.0 : 0.0; // compare la depth stockée à la depth du fragment afin de savoir si la light est visible

    // half vectors, for blinn-phong shading
    vec3 hPointLight = normalize(position + dirToPointLight);
    vec3 hDirLight = normalize(position + uDirectionalLightDir);

    float dothPointLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hPointLight));
    float dothDirLight = shininess == 0 ? 1.f : max(0.f, dot(normal, hDirLight));

    if (shininess > 0.f)
    {
        dothPointLight = pow(dothPointLight, shininess);
        dothDirLight = pow(dothDirLight, shininess);
    }

    fFragColor = ka; // OK
    fFragColor += kd * (uDirectionalLightIntensity * max(0.f, dot(normal, uDirectionalLightDir)) + pointLightIncidentLight * max(0., dot(normal, dirToPointLight)));
    fFragColor += ks * (uDirectionalLightIntensity * dothDirLight * dirLightVisibility + pointLightIncidentLight * dothPointLight);
};
