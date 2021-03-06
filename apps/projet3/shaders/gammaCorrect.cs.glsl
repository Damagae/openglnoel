#version 430

uniform float uGammaExponent;

layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform readonly image2D uInputImage;
layout(binding = 1) uniform writeonly image2D uOutputImage;


void main() {

    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    vec3 image = imageLoad(uInputImage, pixelCoords).rgb;
    vec3 outputValue = pow(image, vec3(uGammaExponent));
    imageStore(uOutputImage, pixelCoords, vec4(outputValue, 1.0));

}
